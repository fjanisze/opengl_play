#ifndef RENDERABLE_OBJECT_HPP
#define RENDERABLE_OBJECT_HPP

#include <headers.hpp>
#include <vector>
#include <list>

namespace renderable
{

class renderable_object;

/*
 * Renderable objects are classified by
 * priority. The priority influence the
 * precendence of drawing
 */
struct rendr_class
{
	rendr_class() = default;
	rendr_class( std::size_t prio ) :
		priority{ prio } {}
	/*
	 * The lower the better
	 */
	std::size_t priority{ 5 };
	std::vector<renderable_object*> renderables;
};

class renderable_object
{
public:
	static void render_renderables(glm::mat4 view, glm::mat4 projection);
	/*
	 * The priority should be between 1 and 10, a lower number
	 * means higher priority, object wich higher priority
	 * are rendered first
	 */
	static void add_renderable(renderable_object* obj,
					std::size_t priority = 5);
	static bool remove_renderable(renderable_object* obj);

	virtual void set_transformations(glm::mat4 view,glm::mat4 projection);
	virtual void prepare_for_render() {}
	virtual void render() {}
	virtual void clean_after_render() {}
	virtual ~renderable_object() {}
	virtual std::string renderable_nice_name();

	virtual void rotate_object(GLfloat yaw) {}
private:
	static std::map<std::size_t,rendr_class> renderables;
protected:
	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;
};

}

#endif
