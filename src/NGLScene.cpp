#include "NGLScene.h"
#include <QMouseEvent>
#include <QGuiApplication>

#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/NGLStream.h>


NGLScene::NGLScene()
{
  setTitle("Qt5 Euler vs roation matrix functions");

}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(QResizeEvent *_event)
{
  m_win.width=static_cast<int>(_event->size().width()*devicePixelRatio());
  m_win.height=static_cast<int>(_event->size().height()*devicePixelRatio());
}

void NGLScene::resizeGL(int _w , int _h)
{
  m_win.width=static_cast<int>(_w*devicePixelRatio());
  m_win.height=static_cast<int>(_h*devicePixelRatio());
}


void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
#ifndef USINGIOS_
  glEnable(GL_MULTISAMPLE);
#endif
   // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong to save typos
  // in the code create some constexpr
  constexpr auto shaderProgram="Phong";
  constexpr auto vertexShader="PhongVertex";
  constexpr auto fragShader="PhongFragment";
  // create the shader program
  shader->createShaderProgram(shaderProgram);
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader(vertexShader,ngl::ShaderType::VERTEX);
  shader->attachShader(fragShader,ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource(vertexShader,"shaders/PhongVertex.glsl");
  shader->loadShaderSource(fragShader,"shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader(vertexShader);
  shader->compileShader(fragShader);
  // add them to the program
  shader->attachShaderToProgram(shaderProgram,vertexShader);
  shader->attachShaderToProgram(shaderProgram,fragShader);


  // now we have associated that data we can link the shader
  shader->linkProgramObject(shaderProgram);
  // and make it active ready to load values
  (*shader)[shaderProgram]->use();
  // the shader will use the currently active material and light0 so set them
  ngl::Material m(ngl::STDMAT::GOLD);
  // load our material values to the shader into the structure material (see Vertex shader)
  m.loadToShader("material");
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  // first draw a top  persp // front //side
  ngl::Vec3 from(0.0f,0.0f,8.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);
  m_view=ngl::lookAt(from,to,up);
  m_projection=ngl::perspective(45.0f,1024.0f/720.0f,0.1f,100.0f);

  shader->setUniform("viewerPos",from);
  // now create our light that is done after the camera so we can pass the
  // transpose of the projection matrix to the light to do correct eye space
  // transformations
  ngl::Mat4 iv=m_view;
  iv.transpose();
  ngl::Light light(ngl::Vec3(-2,5,2),ngl::Colour(1,1,1,1),ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT );
  light.setTransform(iv);
  // load these values to the shader as well
  light.loadToShader("light");
  startTimer(10);
}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_transform.getMatrix()*m_rotationMatrix;
  MV=  m_view*M;
  MVP= m_projection*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("MV",MV);
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
  shader->setUniform("M",M);
}

void NGLScene::paintGL()
{
  glViewport(0,0,m_win.width,m_win.height);
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();



   // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  // draw
  constexpr float xpos=1.0f;
  m_transform.setPosition(-xpos,2.0f,0.0f);
  m_rotationMatrix.identity();
  m_rotationMatrix.rotateX(m_rotation);
  loadMatricesToShader();
  prim->draw("teapot");

  m_transform.setPosition(xpos,2.0f,0.0f);
  m_rotationMatrix.identity();
  m_rotationMatrix.euler(m_rotation,1.0f,0.0f,0.0f);
  loadMatricesToShader();
  prim->draw("teapot");

  m_transform.setPosition(-xpos,0.0f,0.0f);
  m_rotationMatrix.identity();
  m_rotationMatrix.rotateY(m_rotation);
  loadMatricesToShader();
  prim->draw("teapot");

  m_transform.setPosition(xpos,0.0f,0.0f);
  m_rotationMatrix.identity();
  m_rotationMatrix.euler(m_rotation,0.0f,1.0f,0.0f);
  loadMatricesToShader();
  prim->draw("teapot");

  m_transform.setPosition(-xpos,-2.0,0.0f);
  m_rotationMatrix.identity();
  m_rotationMatrix.rotateZ(m_rotation);
  loadMatricesToShader();
  prim->draw("teapot");

  m_transform.setPosition(xpos,-2.0f,0.0f);
  m_rotationMatrix.identity();
  m_rotationMatrix.euler(m_rotation,0.0f,0.0f,1.0f);
  loadMatricesToShader();
  prim->draw("teapot");

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{

}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{


  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quit
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
#ifndef USINGIOS_

  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
#endif
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  default : break;
  }
 update();
}

void NGLScene::timerEvent(QTimerEvent *)
{
  m_rotation+=1;
  update();
}
