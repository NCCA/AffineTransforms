/// @file NGLScene.cpp
/// @brief basic implementation file for the NGLScene class
#include "NGLScene.h"
#include <iostream>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <array>
#include <QDebug>
#include <QMouseEvent>

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

constexpr auto NormalShader="normalShader";
constexpr auto ColourShader="nglColourShader";
constexpr auto PBR = "PBR";

//----------------------------------------------------------------------------------------------------------------------
NGLScene::NGLScene(QWidget *_parent )
{

  // set this widget to have the initial keyboard focus
  setFocus();
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  this->resize(_parent->size());
  m_drawIndex=6;
  m_drawNormals=false;
  /// set all our matrices to the identity
  m_transform=1.0f;
  m_rotate=1.0f;
  m_translate=1.0f;
  m_scale=1.0f;
  m_normalSize=6.0f;
  m_colour.set(0.5f,0.5f,0.5f);

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
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0.0f,0.0f,8.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);


  m_view=ngl::lookAt(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45.0f,720.0f/576.0f,0.5f,10.0f);

  ngl::VAOPrimitives::createSphere("sphere",1.0f,40.0f);
  ngl::VAOPrimitives::createCylinder("cylinder",0.5f,1.4f,40.0f,40.0f);
  ngl::VAOPrimitives::createCone("cone",0.5f,1.4f,20.0f,20.0f);
  ngl::VAOPrimitives::createDisk("disk",0.5f,40.0f);
  ngl::VAOPrimitives::createTrianglePlane("plane",1.0f,1.0f,10.0f,10.0f,ngl::Vec3(0.0f,1.0f,0.0f));
  ngl::VAOPrimitives::createTorus("torus",0.15f,0.4f,40.0f,40.0f);
  // set the bg colour
  glClearColor(0.5,0.5,0.5,0.0);
  m_axis.reset( new Axis(ColourShader,1.5f));
  // load the normal shader

  ngl::ShaderLib::loadShader(PBR,"shaders/PBRVertex.glsl","shaders/PBRFragment.glsl");
  ngl::ShaderLib::use(PBR);
  ngl::ShaderLib::setUniform( "camPos", from );
  // these are "uniform" so will retain their values
  ngl::ShaderLib::setUniform("lightPosition",0.0f, 2.0f, 2.0f);
  ngl::ShaderLib::setUniform("lightColor",400.0f,400.0f,400.0f);
  ngl::ShaderLib::setUniform("exposure",2.2f);
  ngl::ShaderLib::setUniform("albedo",0.950f, 0.71f, 0.29f);

  ngl::ShaderLib::setUniform("metallic",1.02f);
  ngl::ShaderLib::setUniform("roughness",0.38f);
  ngl::ShaderLib::setUniform("ao",0.2f);


  ngl::ShaderLib::createShaderProgram(NormalShader);
  constexpr auto normalVert="normalVertex";
  constexpr auto normalGeo="normalGeo";
  constexpr auto normalFrag="normalFrag";

  ngl::ShaderLib::attachShader(normalVert,ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader(normalFrag,ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource(normalVert,"shaders/normalVertex.glsl");
  ngl::ShaderLib::loadShaderSource(normalFrag,"shaders/normalFragment.glsl");

  ngl::ShaderLib::compileShader(normalVert);
  ngl::ShaderLib::compileShader(normalFrag);
  ngl::ShaderLib::attachShaderToProgram(NormalShader,normalVert);
  ngl::ShaderLib::attachShaderToProgram(NormalShader,normalFrag);

  ngl::ShaderLib::attachShader(normalGeo,ngl::ShaderType::GEOMETRY);
  ngl::ShaderLib::loadShaderSource(normalGeo,"shaders/normalGeo.glsl");
  ngl::ShaderLib::compileShader(normalGeo);
  ngl::ShaderLib::attachShaderToProgram(NormalShader,normalGeo);

  ngl::ShaderLib::linkProgramObject(NormalShader);
  ngl::ShaderLib::use(NormalShader);
  // now pass the modelView and projection values to the shader
  ngl::ShaderLib::setUniform("normalSize",0.1f);
  ngl::ShaderLib::setUniform("vertNormalColour",1.0f,1.0f,0.0f,1.0f);
  ngl::ShaderLib::setUniform("faceNormalColour",1.0f,0.0f,0.0f,1.0f);

  ngl::ShaderLib::setUniform("drawFaceNormals",true);
  ngl::ShaderLib::setUniform("drawVertexNormals",true);
}

//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget has been resized.
// The new size is passed in width and height.
void NGLScene::resizeGL(int _w, int _h )
{
  glViewport(0,0,_w,_h);
  m_project=ngl::perspective(45.0f,static_cast<float>(_w)/_h,0.05f,450.0f);

}

void NGLScene::loadMatricesToShader( )
{
  ngl::ShaderLib::use("PBR");
  struct transform
  {
    ngl::Mat4 MVP;
    ngl::Mat4 normalMatrix;
    ngl::Mat4 M;
  };

   transform t;
   t.M=m_mouseGlobalTX*m_transform;

   t.MVP=m_project*m_view*t.M;
   t.normalMatrix=t.M;
   t.normalMatrix.inverse().transpose();
   ngl::ShaderLib::setUniformBuffer("TransformUBO",sizeof(transform),&t.MVP.m_00);
   ngl::ShaderLib::setUniform("albedo",m_colour);


}
//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget needs to be painted.
// this is our main drawing routine
void NGLScene::paintGL()
{
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
  ngl::ShaderLib::use(PBR);


  // Rotation based on the mouse position for our global transform
    ngl::Mat4 rotX;
    ngl::Mat4 rotY;
    // create the rotation matrices
    rotX.rotateX(m_win.spinXFace);
    rotY.rotateY(m_win.spinYFace);
    // multiply the rotations
    m_mouseGlobalTX=rotY*rotX;
    // add the translations
    m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
    m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
    m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;


    loadMatricesToShader();
    
    if( m_wireframe)
    {
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    }
    else
    {
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    }

    ngl::VAOPrimitives::draw( s_vboNames[m_drawIndex]);
    if(m_drawNormals)
    {
      ngl::ShaderLib::use(NormalShader);
      ngl::Mat4 MV;
      ngl::Mat4 MVP;

      MV=m_view*m_mouseGlobalTX*m_transform;
      MVP=m_project*MV;
      ngl::ShaderLib::setUniform("MVP",MVP);
      ngl::ShaderLib::setUniform("normalSize",m_normalSize/10.0f);

      ngl::VAOPrimitives::draw( s_vboNames[m_drawIndex]);
    }
  m_axis->draw(m_view,m_project,m_mouseGlobalTX);
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent( QMouseEvent* _event )
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if ( m_win.rotate && _event->buttons() == Qt::LeftButton )
  {
    int diffx = _event->x() - m_win.origX;
    int diffy = _event->y() - m_win.origY;
    m_win.spinXFace += static_cast<int>( 0.5f * diffy );
    m_win.spinYFace += static_cast<int>( 0.5f * diffx );
    m_win.origX = _event->x();
    m_win.origY = _event->y();
    update();
  }
  // right mouse translate code
  else if ( m_win.translate && _event->buttons() == Qt::RightButton )
  {
    int diffX      = static_cast<int>( _event->x() - m_win.origXPos );
    int diffY      = static_cast<int>( _event->y() - m_win.origYPos );
    m_win.origXPos = _event->x();
    m_win.origYPos = _event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent( QMouseEvent* _event )
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.origX  = _event->x();
    m_win.origY  = _event->y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if ( _event->button() == Qt::RightButton )
  {
    m_win.origXPos  = _event->x();
    m_win.origYPos  = _event->y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent( QMouseEvent* _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if ( _event->button() == Qt::RightButton )
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent( QWheelEvent* _event )
{

  // check the diff of the wheel position (0 means no change)
  if ( _event->angleDelta().x() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->angleDelta().x() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
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

  //m_material.setDiffuse(m_colour);
  //m_material.loadToShader("material");
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
  m_win.spinXFace=0;
  m_win.spinYFace=0;
  m_win.origX=0;
  m_win.origY=0;
  update();
}
//----------------------------------------------------------------------------------------------------------------------
