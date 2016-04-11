/// @file NGLScene.cpp
/// @brief basic implementation file for the NGLScene class
#include "NGLScene.h"
#include <iostream>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <array>
#include <QDebug>
#include <QMouseEvent>
constexpr float INCREMENT=0.01f;
constexpr float ZOOM=0.1f;

const std::array<std::string,17> s_vboNames={
{
  "sphere",
  "cylinder",
  "cone",
  "disk",
  "plane",
  "torus",
  "teapot",
  "octahedron",
  "dodecahedron",
  "icosahedron",
  "tetrahedron",
  "football",
  "cube",
  "troll",
  "buddah",
  "dragon",
  "bunny"
  }
};

constexpr auto PhongShader="PhongShader";
constexpr auto NormalShader="normalShader";
constexpr auto ColourShader="nglColourShader";


//----------------------------------------------------------------------------------------------------------------------
NGLScene::NGLScene(QWidget *_parent )
{

  // set this widget to have the initial keyboard focus
  setFocus();
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  this->resize(_parent->size());
  // Now set the initial NGLScene attributes to default values
  // Roate is false
  m_rotateActive=false;
  m_translateActive=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  m_origX=0;
  m_origY=0;
  m_drawIndex=6;
  m_drawNormals=false;
  /// set all our matrices to the identity
  m_transform=1.0f;
  m_rotate=1.0f;
  m_translate=1.0f;
  m_scale=1.0f;
  m_normalSize=6.0f;
  m_colour.set(0.5f,0.5f,0.5f);
  m_material.setDiffuse(m_colour);
  m_material.setSpecular(ngl::Colour(0.2f,0.2f,0.2f));
  m_material.setAmbient(ngl::Colour());
  m_material.setSpecularExponent(20.0f);
  m_material.setRoughness(0.0f);
  m_matrixOrder=NGLScene::MatrixOrder::RTS;
  m_euler=1.0f;
  m_modelPos.set(0.0f,0.0f,0.0f);
}

// This virtual function is called once before the first call to paintGL() or resizeGL(),
//and then once whenever the widget has been assigned a new QGLContext.
// This function should set up any required OpenGL context rendering flags, defining display lists, etc.

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::initializeGL()
{
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0.0f,0.0f,8.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);


  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45.0f,720.0f/576.0f,0.5f,10.0f);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  // we are creating a shader called Phong
  shader->createShaderProgram(PhongShader);

  // now we are going to create empty shaders for Frag and Vert
  // use thes string to save typos
  constexpr auto vertexShader="PhongVertex";
  constexpr auto fragShader="PhongFragment";

  shader->attachShader(vertexShader,ngl::ShaderType::VERTEX);
  shader->attachShader(fragShader,ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource(vertexShader,"shaders/PhongVertex.glsl");
  shader->loadShaderSource(fragShader,"shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader(vertexShader);
  shader->compileShader(fragShader);
  // add them to the program
  shader->attachShaderToProgram(PhongShader,vertexShader);
  shader->attachShaderToProgram(PhongShader,fragShader);

  // now we have associated this data we can link the shader
  shader->linkProgramObject(PhongShader);
  // and make it active ready to load values
  (*shader)[PhongShader]->use();

  // now create our light this is done after the camera so we can pass the
  // transpose of the projection matrix to the light to do correct eye space
  // transformations
  ngl::Mat4 iv=m_cam.getProjectionMatrix();
  iv.transpose();
  ngl::Light light(ngl::Vec3(-2.0f,2.0f,-2.0f),ngl::Colour(1.0f,1.0f,1.0f,1.0f),ngl::Colour(1.0f,1.0f,1.0f,1.0f),ngl::LightModes::POINTLIGHT);
  light.setTransform(iv);
  // load these values to the shader as well
  light.loadToShader("light");
  /// now create our primitives for drawing later

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",1.0f,40.0f);
  prim->createCylinder("cylinder",0.5f,1.4f,40.0f,40.0f);
  prim->createCone("cone",0.5f,1.4f,20.0f,20.0f);
  prim->createDisk("disk",0.5f,40.0f);
  prim->createTrianglePlane("plane",1.0f,1.0f,10.0f,10.0f,ngl::Vec3(0.0f,1.0f,0.0f));
  prim->createTorus("torus",0.15f,0.4f,40.0f,40.0f);
  // set the bg colour
  glClearColor(0.5,0.5,0.5,0.0);
  m_axis.reset( new Axis(ColourShader,1.5f));
  m_axis->setCam(&m_cam);
  // load the normal shader
  shader->createShaderProgram(NormalShader);
  constexpr auto normalVert="normalVertex";
  constexpr auto normalGeo="normalGeo";
  constexpr auto normalFrag="normalFrag";

  shader->attachShader(normalVert,ngl::ShaderType::VERTEX);
  shader->attachShader(normalFrag,ngl::ShaderType::FRAGMENT);
  shader->loadShaderSource(normalVert,"shaders/normalVertex.glsl");
  shader->loadShaderSource(normalFrag,"shaders/normalFragment.glsl");

  shader->compileShader(normalVert);
  shader->compileShader(normalFrag);
  shader->attachShaderToProgram(NormalShader,normalVert);
  shader->attachShaderToProgram(NormalShader,normalFrag);

  shader->attachShader(normalGeo,ngl::ShaderType::GEOMETRY);
  shader->loadShaderSource(normalGeo,"shaders/normalGeo.glsl");
  shader->compileShader(normalGeo);
  shader->attachShaderToProgram(NormalShader,normalGeo);

  shader->linkProgramObject(NormalShader);
  shader->use(NormalShader);
  // now pass the modelView and projection values to the shader
  shader->setUniform("normalSize",0.1f);
  shader->setUniform("vertNormalColour",1.0f,1.0f,0.0f,1.0f);
  shader->setUniform("faceNormalColour",1.0f,0.0f,0.0f,1.0f);

  shader->setShaderParam1i("drawFaceNormals",true);
  shader->setShaderParam1i("drawVertexNormals",true);
}

//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget has been resized.
// The new size is passed in width and height.
void NGLScene::resizeGL(int _w, int _h )
{
  glViewport(0,0,_w,_h);
  m_cam.setShape(45.0f,static_cast<float>(_w)/_h,0.05f,450.0f);

}

void NGLScene::loadMatricesToShader( )
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)[PhongShader]->use();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_transform*m_mouseGlobalTX;
  MV=M*m_cam.getViewMatrix();
  MVP=  MV*m_cam.getProjectionMatrix();
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);
}
//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget needs to be painted.
// this is our main drawing routine
void NGLScene::paintGL()
{
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Rotation based on the mouse position for our global
  // transform

  m_transform.identity();

  if (m_matrixOrder == NGLScene::MatrixOrder::RTS)
  {
    m_transform=m_rotate*m_translate*m_scale;
  }

  else if (m_matrixOrder == NGLScene::MatrixOrder::TRS)
  {
    m_transform=m_translate*m_rotate*m_scale;
  }
  else if (m_matrixOrder == NGLScene::MatrixOrder::EULERTS)
  {
    m_transform=m_translate*m_euler*m_scale;
  }
  else if (m_matrixOrder == NGLScene::MatrixOrder::TEULERS)
  {
    m_transform=m_euler*m_translate*m_scale;
  }

  else if (m_matrixOrder == NGLScene::MatrixOrder::GIMBALLOCK )
  {
    m_transform=m_translate*m_gimbal*m_scale;
  }
  emit matrixDirty(m_transform);
  // now set this value in the shader for the current ModelMatrix
  (*shader)[PhongShader]->use();

  m_material.loadToShader("material");


  // Rotation based on the mouse position for our global transform
    ngl::Mat4 rotX;
    ngl::Mat4 rotY;
    // create the rotation matrices
    rotX.rotateX(m_spinXFace);
    rotY.rotateY(m_spinYFace);
    // multiply the rotations
    m_mouseGlobalTX=rotY*rotX;
    // add the translations
    m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
    m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
    m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;


    loadMatricesToShader();
    // get the VBO instance and draw the built in teapot

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    if( m_wireframe)
    {
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    }
    else
    {
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    }

    prim->draw( s_vboNames[m_drawIndex]);
    if(m_drawNormals)
    {
      (*shader)[NormalShader]->use();
      ngl::Mat4 MV;
      ngl::Mat4 MVP;

      MV=m_transform*m_mouseGlobalTX* m_cam.getViewMatrix();
      MVP=MV*m_cam.getProjectionMatrix();
      shader->setUniform("MVP",MVP);
      shader->setUniform("normalSize",m_normalSize/10.0f);

      prim->draw( s_vboNames[m_drawIndex]);
    }
  m_axis->draw(m_mouseGlobalTX);
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event  )
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotateActive && _event->buttons() == Qt::LeftButton)
  {
    m_spinYFace = ( m_spinYFace + (_event->x() - m_origX) ) % 360 ;
    m_spinXFace = ( m_spinXFace + (_event->y() - m_origY) ) % 360 ;
    m_origX = _event->x();
    m_origY = _event->y();

    update();

  }
  // right mouse translate code
  else if(m_translateActive && _event->buttons() == Qt::RightButton)
  {
    int diffX = static_cast<int>(_event->x() - m_origXPos);
    int diffY = static_cast<int>(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();

  }

}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent (QMouseEvent * _event )
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotateActive =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translateActive=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent (QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotateActive=false;
  }
  // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translateActive=false;
  }
}

void NGLScene::wheelEvent(QWheelEvent *_event)
{

  // check the diff of the wheel position (0 means no change)
  if(_event->delta() > 0)
  {
    m_modelPos.m_z+=ZOOM;
  }
  else if(_event->delta() <0 )
  {
    m_modelPos.m_z-=ZOOM;
  }
  update();
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::vboChanged( int _index  )
{
  m_drawIndex=_index;
  update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::toggleNormals(bool _value )
{
  m_drawNormals=_value;
  update();
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::setNormalSize(int _value)
{
  m_normalSize=_value;
  update();
}
//----------------------------------------------------------------------------------------------------------------------
void NGLScene::setScale(float _x, float _y,float _z )
{
  m_scale.scale(_x,_y,_z);
  update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::setTranslate(float _x, float _y, float _z )
{

  m_translate.translate(_x,_y,_z);
  update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::setRotate(float _x,  float _y, float _z  )
{
  ngl::Mat4 rx=1.0f;
  ngl::Mat4 ry=1.0f;
  ngl::Mat4 rz=1.0f;
  rx.rotateX(_x);
  ry.rotateY(_y);
  rz.rotateZ(_z);
  m_rotate=rz*ry*rx;
  // now for the incorrect gimbal 1
  m_gimbal.identity();
  m_gimbal.rotateX(_x);
  m_gimbal.rotateY(_y);
  m_gimbal.rotateZ(_z);

  update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::setColour( float _r,   float _g,  float _b  )
{
  m_colour.set(_r,_g,_b);
  m_material.setDiffuse(m_colour);
  m_material.loadToShader("material");
  update();
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::setMatrixOrder(int _index  )
{
  switch(_index)
  {
    case 0 : { m_matrixOrder=NGLScene::MatrixOrder::RTS;   break; }
    case 1 : { m_matrixOrder=NGLScene::MatrixOrder::TRS;   break; }
    case 2 : { m_matrixOrder=NGLScene::MatrixOrder::GIMBALLOCK; break; }
    case 3 : { m_matrixOrder=NGLScene::MatrixOrder::EULERTS; break; }
    case 4 : { m_matrixOrder=NGLScene::MatrixOrder::TEULERS; break; }
    default : break;
  }
update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::setEuler(float _angle, float _x,  float _y,  float _z )
{
  m_euler=1.0f;
  m_euler.euler(_angle,_x,_y,_z);
  update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::resetMouse()
{
  m_spinXFace=0;
  m_spinYFace=0;
  m_origX=0;
  m_origY=0;
  update();
}
//----------------------------------------------------------------------------------------------------------------------
