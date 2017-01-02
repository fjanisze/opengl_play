#include <renderable_object.hpp>

namespace renderable
{

std::vector<renderable_object*> renderable_object::renderables;

void renderable_object::render_renderables(glm::mat4 model,
								glm::mat4 view,
								glm::mat4 projection)
{
	for(auto& obj : renderables) {
		obj->set_transformations(model,view,projection);
		obj->prepare_for_render();
		obj->render();
		obj->clean_after_render();
	}
}

void renderable_object::add_renderable(renderable_object * object)
{
	renderables.emplace_back(object);
}

}
