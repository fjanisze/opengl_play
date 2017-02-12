#include <models.hpp>
#include <logger/logger.hpp>

namespace models
{

//////////////////////////////////////
/// my_mesh implementation
/////////////////////////////////////

my_mesh::my_mesh(shaders::my_small_shaders *shad,
			vertices_ptr vertx,
			indices_ptr indx,
			textures_ptr texts) :
	shader{ shad },
	vertices{ std::move( vertx ) },
	indices{ std::move( indx ) },
	textures{ std::move( texts ) }
{
	LOG1("Creating my_mesh, data: ",
		 vertices->size(),",",indices->size(),
		 ",",textures->size());
	setup_mesh();
}

void my_mesh::setup_mesh()
{
	LOG1("my_mesh::setup_mesh: Setup of all the OpenGL buffers");
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER,
			vertices->size() * sizeof(vertex_t),
			&vertices->at(0),
			GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			indices->size() * sizeof(GLuint),
			&indices->at(0),
			GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
						  (GLvoid*)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
						  (GLvoid*)offsetof(vertex_t, normal));
	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
						  (GLvoid*)offsetof(vertex_t, texture_coord));

	glBindVertexArray(0);
}

void my_mesh::render()
{
	//TODO: Missing textures..
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

//////////////////////////////////////
/// my_model implementation
/////////////////////////////////////

my_model::my_model(shaders::my_small_shaders *shad,
				const std::string &model_path) :
	shader{ shad },
	model_path{ model_path }
{
	LOG1("Creating a new my_model. Model path: ",
		 model_path.c_str());
	load_model();
}

void my_model::render()
{
	for( auto& mesh : meshes ) {
		mesh->render();
	}
}

bool my_model::load_model()
{
	LOG1("my_model::load_model: Loading ",
		 model_path.c_str());
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(model_path,
							aiProcess_Triangulate | //Transform all model primitives to triangles
							aiProcess_FlipUVs);	//Flip the text coordinates on Y

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		ERR("Unable to load the scene, ERROR: ",
			import.GetErrorString());
		return false;
	}
	model_directory = model_path.substr(0, model_path.find_last_of('/'));

	process_model(scene->mRootNode, scene);
	return true;
}

void my_model::process_model(aiNode *node,
					const aiScene *scene)
{
	LOG1("Processing loaded model, ",
		 model_path.c_str());
	// Process all the node's meshes (if any)
	for(GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		// Create a new my_mesh
		mesh_ptr new_mesh = process_mesh(mesh,scene);
		meshes.push_back( std::forward<mesh_ptr>( new_mesh ) );
	}
	// Then do the same for each of its children
	for(GLuint i = 0; i < node->mNumChildren; i++)
	{
		process_model(node->mChildren[i], scene);
	}
}

my_model::mesh_ptr my_model::process_mesh(aiMesh *mesh,
										  const aiScene *scene)
{
	my_mesh::vertices_ptr vertices = std::make_unique<std::vector<vertex_t>>();
	my_mesh::indices_ptr  indices = std::make_unique<std::vector<GLuint>>();
	my_mesh::textures_ptr textures = std::make_unique<std::vector<texture_t>>();

	// Walk through each of the mesh's vertices
	for(GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		vertex_t vertex;
		// Positions
		vertex.coordinate.x = mesh->mVertices[i].x;
		vertex.coordinate.y = mesh->mVertices[i].y;
		vertex.coordinate.z = mesh->mVertices[i].z;
		// Normals
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		// Texture Coordinates
		if(mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.texture_coord.x = mesh->mTextureCoords[0][i].x;
			vertex.texture_coord.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texture_coord = glm::vec2(0.0f, 0.0f);
		vertices->push_back(vertex);
	}


	// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for(GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for(GLuint j = 0; j < face.mNumIndices; j++)
			indices->push_back(face.mIndices[j]);
	}
/*
	// Process materials
	if(mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
		// Same applies to other texture as the following list summarizes:
		// Diffuse: texture_diffuseN
		// Specular: texture_specularN
		// Normal: texture_normalN

		// 1. Diffuse maps
		vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. Specular maps
		vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}
*/
	// Return a mesh object created from the extracted mesh data
	return std::make_unique<my_mesh>(shader,
									 std::forward<my_mesh::vertices_ptr>( vertices ),
									 std::forward<my_mesh::indices_ptr>( indices ),
									 std::forward<my_mesh::textures_ptr>( textures) );
}

}
