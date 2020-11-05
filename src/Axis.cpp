#include "Axis.h"
#include <QDebug>

//----------------------------------------------------------------------------------------------------------------------
Axis::Axis(std::string _shaderName, ngl::Real _scale )
{
  m_shaderName=_shaderName;
  m_scale=_scale;
  ngl::VAOPrimitives::createCylinder("nglAXISCylinder",0.02f,2,60,60);
  ngl::VAOPrimitives::createCone("nglAXISCone",0.05f,0.2f,30,30);
}

void Axis::loadMatricesToShader(const ngl::Mat4 &_view, const ngl::Mat4 &_project)
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat4 M;
  M=m_globalMouseTx*m_transform.getMatrix();
  MV=  _view*M;
  MVP= _project*MV;
 // MVP= m_transform.getMatrix() * m_cam->getVPMatrix();
  ngl::ShaderLib::setUniform("MVP",MVP);
}

//----------------------------------------------------------------------------------------------------------------------
void Axis::draw(const ngl::Mat4 &_view, const ngl::Mat4 &_project, const ngl::Mat4 &_globalTx )
{
  m_globalMouseTx=_globalTx;
  ngl::ShaderLib::use(m_shaderName);
  // Rotation based on the mouse position for our global
  // transform

  ngl::ShaderLib::setUniform("Colour",1.0f,0.0f,0.0f,1.0f);
  m_transform.setScale(m_scale,m_scale,m_scale*2);
  m_transform.setPosition(ngl::Vec3(m_scale,0,0));
  m_transform.setRotation(0,90,0);
  loadMatricesToShader(_view,_project);
  ngl::VAOPrimitives::draw("nglAXISCylinder");

  m_transform.setPosition(ngl::Vec3(m_scale,0,0));
  m_transform.setRotation(0,90,0);
  loadMatricesToShader(_view,_project);
  ngl::VAOPrimitives::draw("nglAXISCone");
  m_transform.setPosition(ngl::Vec3(-m_scale,0,0));
  m_transform.setRotation(0,-90,0);
  loadMatricesToShader(_view,_project);
  ngl::VAOPrimitives::draw("nglAXISCone");

  // y axis
   ngl::ShaderLib::setUniform("Colour",0.0f,1.0f,0.0f,1.0f);
   m_transform.setScale(m_scale,m_scale,m_scale*2);
   m_transform.setPosition(ngl::Vec3(0,-m_scale,0));
   m_transform.setRotation(90,0,0);
   loadMatricesToShader(_view,_project);
   ngl::VAOPrimitives::draw("nglAXISCylinder");

   m_transform.setPosition(ngl::Vec3(0,m_scale,0));
   m_transform.setRotation(-90,0,0);
   loadMatricesToShader(_view,_project);
   ngl::VAOPrimitives::draw("nglAXISCone");
   m_transform.setPosition(ngl::Vec3(0,-m_scale,0));
   m_transform.setRotation(90,0,0);
   loadMatricesToShader(_view,_project);
   ngl::VAOPrimitives::draw("nglAXISCone");

//     // z axis
   ngl::ShaderLib::setUniform("Colour",0.0f,0.0f,1.0f,1.0f);
   m_transform.setScale(m_scale,m_scale,m_scale*2);
   m_transform.setPosition(ngl::Vec3(0,0,m_scale));
   m_transform.setRotation(0,0,-90);
   loadMatricesToShader(_view,_project);
   ngl::VAOPrimitives::draw("nglAXISCylinder");

   m_transform.setPosition(ngl::Vec3(0,0,m_scale));
   loadMatricesToShader(_view,_project);
   ngl::VAOPrimitives::draw("nglAXISCone");
   m_transform.setPosition(ngl::Vec3(0,0,-m_scale));
   m_transform.setRotation(180,0,0);
   loadMatricesToShader(_view,_project);
   ngl::VAOPrimitives::draw("nglAXISCone");

}
