#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>
#include <glm/gtx/string_cast.hpp>
#include "scene_node.h"

namespace game {

SceneNode::SceneNode(const std::string name, const Resource *geometry, const Resource *material){

    // Set name of scene node
    name_ = name;

    // Set geometry
    if (geometry->GetType() == PointSet){
        mode_ = GL_POINTS;
    } else if (geometry->GetType() == Mesh){
        mode_ = GL_TRIANGLES;
    } else {
        throw(std::invalid_argument(std::string("Invalid type of geometry")));
    }

    array_buffer_ = geometry->GetArrayBuffer();
    element_array_buffer_ = geometry->GetElementArrayBuffer();
    size_ = geometry->GetSize();

    // Set material (shader program)
    if (material->GetType() != Material){
        throw(std::invalid_argument(std::string("Invalid type of material")));
    }

    material_ = material->GetResource();

    // Other attributes
    scale_ = glm::vec3(1.0, 1.0, 1.0);
}


SceneNode::~SceneNode(){
}


const std::string SceneNode::GetName(void) const {

    return name_;
}


glm::vec3 SceneNode::GetPosition(void) const {

    return position_;
}


glm::quat SceneNode::GetOrientation(void) const {

    return orientation_;
}


glm::vec3 SceneNode::GetScale(void) const {

    return scale_;
}


void SceneNode::SetPosition(glm::vec3 position){

    position_ = position;
}


void SceneNode::SetOrientation(glm::quat orientation){

    orientation_ = orientation;
}


void SceneNode::SetScale(glm::vec3 scale){

    scale_ = scale;
}


void SceneNode::Translate(glm::vec3 trans){

    position_ += trans;
}


void SceneNode::Rotate(glm::quat rot){

    orientation_ *= rot;
    orientation_ = glm::normalize(orientation_);
}


void SceneNode::Scale(glm::vec3 scale){

    scale_ *= scale;
}


GLenum SceneNode::GetMode(void) const {

    return mode_;
}


GLuint SceneNode::GetArrayBuffer(void) const {

    return array_buffer_;
}


GLuint SceneNode::GetElementArrayBuffer(void) const {

    return element_array_buffer_;
}


GLsizei SceneNode::GetSize(void) const {

    return size_;
}


GLuint SceneNode::GetMaterial(void) const {

    return material_;
}

SceneNode *SceneNode::findIt(std::string node_name) {
	// Find node with the specified name
	for (int i = 0; i < children.size(); i++) {
		if (children[i]->GetName() == node_name) {
			return children[i];
		}
		if (children[i]->GetChildren().size() > 0) {
			SceneNode *a = children[i]->findIt(node_name);
			if (a != NULL) { return a; }
		}
	}
	return NULL;
}

void SceneNode::Draw(Camera *camera){
	if (!live) { liveTime -= 1; }

	if (liveTime < 0) { appear = false; }

	if (appear) {
		// Select proper material (shader program)
		glUseProgram(material_);

		// Set geometry to draw
		glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_);

		// Set globals for camera
		camera->SetupShader(material_);
		// Set world matrix and other shader input variables
		//hierarchy transformations shown in the SetupShader() function
		SetupShader(material_, camera);

		// Draw geometry
		if (mode_ == GL_POINTS) {
			glDrawArrays(mode_, 0, size_);
		}
		else {
			glDrawElements(mode_, size_, GL_UNSIGNED_INT, 0);
		}

		for (int i = 0; i < children.size(); i += 1) {
			children[i]->Draw(camera);
		}
	}
}


void SceneNode::Update(void){

   
}


void SceneNode::SetupShader(GLuint program, Camera* camera){

    // Set attributes for shaders
    GLint vertex_att = glGetAttribLocation(program, "vertex");
    glVertexAttribPointer(vertex_att, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(vertex_att);

    GLint normal_att = glGetAttribLocation(program, "normal");
    glVertexAttribPointer(normal_att, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void *) (3*sizeof(GLfloat)));
    glEnableVertexAttribArray(normal_att);

    GLint color_att = glGetAttribLocation(program, "color");
    glVertexAttribPointer(color_att, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void *) (6*sizeof(GLfloat)));
    glEnableVertexAttribArray(color_att);

    GLint tex_att = glGetAttribLocation(program, "uv");
    glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void *) (9*sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_att);
	
	glm::mat4 transf;

	// World transformation
	glm::mat4 scaling = glm::scale(glm::mat4(1.0), scale_);
	glm::mat4 rotation = glm::mat4_cast(orientation_);
	glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);

	if (parent && parent->GetName() != "Camera") {
		transf = parent->getTran() * translation * rotation * scaling;
		if (name_ == "Canon") { transf = glm::rotate(transf, Rotation, glm::vec3(0.0, 0.0, 1.0)); }
		TransferMatrix = transf;	
	}
	else if (name_ == "Ship_Body") {
		transf = parent->getTran() * translation * rotation * scaling;
		TransferMatrix = transf;
	}
	else if (name_ == "Missile") {
		transf = TransferMatrix * translation;
	}
	else {
		transf = translation * rotation * scaling;
		TransferMatrix = transf;
	}
	
    GLint world_mat = glGetUniformLocation(program, "world_mat");
    glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(transf));

    // Timer
    GLint timer_var = glGetUniformLocation(program, "timer");
    double current_time = glfwGetTime();
    glUniform1f(timer_var, (float) current_time);
}

} // namespace game;
