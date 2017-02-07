#ifndef RENDERABLE_OBJECT_HPP
#define RENDERABLE_OBJECT_HPP

#include <headers.hpp>
#include <vector>

namespace renderable
{

class renderable_object
{
	static std::vector<renderable_object*> renderables;
public:
	static void render_renderables(glm::mat4 view, glm::mat4 projection);
	void add_renderable(renderable_object * object);
	bool remove_renderable(renderable_object* object);

	virtual void set_transformations(glm::mat4 view,glm::mat4 projection) {}
	virtual void prepare_for_render() {}
	virtual void render() {}
	virtual void clean_after_render() {}
	virtual ~renderable_object() {}

	virtual void rotate_object(GLfloat yaw) {}
};

}

#endif
