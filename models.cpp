#include <models.hpp>
#include <logger/logger.hpp>
#include <thread>
#include <future>
#include <functional>
#include <chrono>
#include <mutex>

namespace models
{

//////////////////////////////////////
/// my_mesh implementation
/////////////////////////////////////

my_mesh::my_mesh(vertices_ptr vertx,
                 indices_ptr indx,
                 textures_ptr texts) :
    vertices{ std::move( vertx ) },
    indices{ std::move( indx ) },
    textures{ std::move( texts ) }
{
    LOG1("Creating my_mesh. VRTX:",
         vertices->size(),", IDX:",indices->size(),
         ", TXT",textures->size());
    setup_mesh();
}

void my_mesh::setup_mesh()
{
    LOG3("Setup of all the OpenGL buffers");
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
    // Vertex Texture Coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                          (GLvoid*)offsetof(vertex_t, texture_coord));
    // Vertex Normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                          (GLvoid*)offsetof(vertex_t, normal));

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
}

void my_mesh::render(shaders::my_small_shaders* shader)
{
    glBindVertexArray(VAO);
    GLuint current_unit = 0;
    GLuint diffuse_nr = 1;
    GLuint specular_nr = 1;

    for( auto& current_tex : *textures )
    {
        glActiveTexture(GL_TEXTURE0 + current_unit);
        std::string name;
        if( current_tex.type == texture_type::diffuse )
        {
            name = "loaded_texture" + std::to_string(diffuse_nr++);
        }
        else if( current_tex.type == texture_type::specular )
        {
            name = "loaded_texture_specular_map" + std::to_string(specular_nr++);
        }
        GLint map = glGetUniformLocation(shader->get_program(),
                                         name.c_str());
        if( map >= 0 ) {
            glUniform1f(map,current_unit);
        } else {
            ERR("Unable to setup the texture unit! ",
                name.c_str());
        }

        glBindTexture(GL_TEXTURE_2D, current_tex.id);
        ++current_unit;
    }
    glActiveTexture(GL_TEXTURE0);

    glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//////////////////////////////////////
/// model_loader and my_model implementation
/////////////////////////////////////

model_loader::model_loader(const std::string &path,
                           z_axis revert_z) :
    model_path{ path },
    revert_z_axis{ revert_z == z_axis::normal  },
    model_height{ 0 }
{
}

bool model_loader::load_model()
{
    LOG3("Loading: ",
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

my_mesh::meshes &model_loader::get_mesh()
{
    return meshes;
}

GLfloat model_loader::get_model_height()
{
    return model_height;
}


void model_loader::process_model(aiNode *node,
                                 const aiScene *scene)
{
    LOG3("Processing loaded model, ",
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
    LOG3("Processing childs: ", node->mNumChildren);
    for(GLuint i = 0; i < node->mNumChildren; i++)
    {
        process_model(node->mChildren[i], scene);
    }
}

mesh_ptr model_loader::process_mesh(aiMesh *mesh,
                                    const aiScene *scene)
{
    my_mesh::vertices_ptr vertices = std::make_unique<std::vector<vertex_t>>();
    my_mesh::indices_ptr  indices = std::make_unique<std::vector<GLuint>>();
    my_mesh::textures_ptr textures;

    // Walk through each of the mesh's vertices
    for(GLuint i = 0; i < mesh->mNumVertices; i++)
    {
        vertex_t vertex;
        // Positions
        vertex.coordinate.x = mesh->mVertices[i].x;
        vertex.coordinate.y = mesh->mVertices[i].z;
        vertex.coordinate.z = -(revert_z_axis ? -1 : 1 ) * mesh->mVertices[i].y;
        model_height = std::max( model_height, vertex.coordinate.z );
        // Normals
        if( mesh->mNormals ) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].z;
            vertex.normal.z = -(revert_z_axis ? -1 : 1 ) * mesh->mNormals[i].y;
        }
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

    // Process materials
    if(mesh->mMaterialIndex >= 0)
    {
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        auto diffuse_maps = process_texture(material,
                                            aiTextureType_DIFFUSE);
        auto specular_maps = process_texture(material,
                                             aiTextureType_SPECULAR);

        diffuse_maps->insert(diffuse_maps->end(),
                             specular_maps->begin(),
                             specular_maps->end());
        textures = std::move( diffuse_maps );
    }
    // Return a mesh object created from the extracted mesh data
    return std::make_unique<my_mesh>(std::forward<my_mesh::vertices_ptr>( vertices ),
                                     std::forward<my_mesh::indices_ptr>( indices ),
                                     std::forward<my_mesh::textures_ptr>( textures ) );
    return nullptr;
}

my_mesh::textures_ptr model_loader::process_texture(const aiMaterial *material,
                                                    aiTextureType type)
{
    my_mesh::textures_ptr textures = std::make_unique<std::vector<texture_t>>();
    for(GLuint i{ 0 } ; i < material->GetTextureCount(type) ; ++i ) {
        texture_t texture;
        aiString path;
        material->GetTexture( type, i, &path );
        texture.type = map_texture_type( type );
        std::string filename = path.C_Str();
        std::size_t pos = filename.find_last_of("\\/");
        if( pos != std::string::npos )
            filename = filename.substr(pos + 1);
        texture.id = load_texture(
                "../models/textures/" + filename);
        texture.path = path.C_Str();
        textures->push_back(texture);
    }
    return std::move( textures );
}

my_model::my_model(const std::string &model_path,
                   const glm::vec4 &def_object_color,
                   z_axis revert_z) :
    model_loader( model_path, revert_z )
{
    LOG3("Creating a new my_model. Model path: ",
         model_path.c_str());

    default_color = def_object_color;
    load_model();
}

}
