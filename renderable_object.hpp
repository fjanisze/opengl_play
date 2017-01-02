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
	static void render_renderables(glm::mat4 model,glm::mat4 view,glm::mat4 projection);
	void add_renderable(renderable_object * object);

	virtual void set_transformations(glm::mat4 model,glm::mat4 view,glm::mat4 projection) = 0;
	virtual void prepare_for_render() = 0;
	virtual void render() = 0;
	virtual void clean_after_render() = 0;
	virtual ~renderable_object() {}
};

}

#endif
