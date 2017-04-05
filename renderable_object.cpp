#include <renderable_object.hpp>
#include <algorithm>
#include <iostream>
#include <logger/logger.hpp>

namespace renderable
{

std::map<std::size_t,rendr_class> renderable_object::renderables;

void renderable_object::render_renderables(glm::mat4 view,
                                           glm::mat4 projection)
{
    for(auto&& entry : renderables) {
        for(auto&& obj:entry.second.renderables) {
            obj->render_me( view, projection );
        }
    }
}

void renderable_object::render_me(glm::mat4 view,
                                  glm::mat4 projection)
{
    set_transformations(view,projection);
    prepare_for_render();
    render();
    clean_after_render();
}

bool renderable_object::add_renderable(renderable_object * object,
                                       std::size_t priority)
{
    LOG1("Adding new renderable: ",
         object->renderable_nice_name());
    for( auto&& entry : renderables ) {
        for( auto&& obj : entry.second.renderables ) {
            if( obj == object ) {
                WARN1("Failed! The object already exist, with priority ",
                      entry.second.priority);
                return false;
            }
        }
    }
    renderables[ priority ].renderables.push_back( object );
    return true;
}

bool renderable_object::remove_renderable(renderable_object *object)
{
    for( auto&& elem : renderables ) {
        auto it = std::find(elem.second.renderables.begin(),
                            elem.second.renderables.end(),
                            object);
        if( it != elem.second.renderables.end() ) {
            elem.second.renderables.erase(it);
            return true;
        }
    }
    return false;
}

void renderable_object::set_transformations(glm::mat4 view,glm::mat4 projection)
{
    projection_matrix = projection;
    view_matrix = view;
}

std::string renderable_object::renderable_nice_name()
{
    return "(nice name not provided)";
}

}
