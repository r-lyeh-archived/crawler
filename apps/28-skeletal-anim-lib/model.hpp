#ifndef MODEL_HPP
#define	MODEL_HPP

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

//For the draw functions:
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

//Defines how to write a new Material class for Model and SkeletalAnimationModel.
//Especially how to handle textures may change depending on the multimedia library used.
//Used as default class if your model does not contain any specific materials or textures.
class Material {
public:
    Material() {}
    Material(const aiMaterial* material) {}
    virtual void bindTexture(aiTextureType textureType, unsigned int textureId) const {}
    virtual bool texture() const {return false;}
};

class Mesh {
public:
    std::vector<aiVector3D> vertices;
    std::vector<aiVector3D> normals;
    std::vector<aiVector2D> textureCoords;

    std::vector<aiFace> faces;

    //In AssImp: one material per mesh
    unsigned int materialId;
};

template<class MaterialType=Material, class MeshType=Mesh>
class Model {
public:
    std::vector<MeshType> meshes;
    std::vector<MaterialType> materials;

    //Draws the given mesh.
    //Currently only supports 1 diffuse texture per material
    virtual void drawMesh(const MeshType& mesh) const {
        bool texture=false;
        if(this->materials[mesh.materialId].texture())
            texture=true;

        //If material has texture, use only the first one (assumes one diffuse texture per material).
        //However, if you want to use more than one texture per mesh, read: http://assimp.sourceforge.net/lib_html/materials.html
        if(texture) {
            this->materials[mesh.materialId].bindTexture(aiTextureType_DIFFUSE, 0);
        }

        glBegin(GL_TRIANGLES);
        for(auto& face: mesh.faces) {
            glNormal3f(mesh.normals[face.mIndices[0]].x, mesh.normals[face.mIndices[0]].y, mesh.normals[face.mIndices[0]].z);
            if(texture)
                glTexCoord2f(mesh.textureCoords[face.mIndices[0]].x, mesh.textureCoords[face.mIndices[0]].y);
            glVertex3f(mesh.vertices[face.mIndices[0]].x, mesh.vertices[face.mIndices[0]].y, mesh.vertices[face.mIndices[0]].z);

            glNormal3f(mesh.normals[face.mIndices[1]].x, mesh.normals[face.mIndices[1]].y, mesh.normals[face.mIndices[1]].z);
            if(texture)
                glTexCoord2f(mesh.textureCoords[face.mIndices[1]].x, mesh.textureCoords[face.mIndices[1]].y);                    
            glVertex3f(mesh.vertices[face.mIndices[1]].x, mesh.vertices[face.mIndices[1]].y, mesh.vertices[face.mIndices[1]].z);

            glNormal3f(mesh.normals[face.mIndices[2]].x, mesh.normals[face.mIndices[2]].y, mesh.normals[face.mIndices[2]].z);
            if(texture)
                glTexCoord2f(mesh.textureCoords[face.mIndices[2]].x, mesh.textureCoords[face.mIndices[2]].y);
            glVertex3f(mesh.vertices[face.mIndices[2]].x, mesh.vertices[face.mIndices[2]].y, mesh.vertices[face.mIndices[2]].z);
        }
        glEnd();
    }

    //Convenient function to draw all the meshes. 
    void draw() const {
        for(auto& mesh: meshes) {
            drawMesh(mesh);
        }
    }

    virtual void read(const std::string& filename, unsigned int assimpImporterFlags=aiProcessPreset_TargetRealtime_Fast) {
        Assimp::Importer importer;

        const aiScene *scene = importer.ReadFile(filename, assimpImporterFlags);
        
        if(scene)
            read(scene);
    }

protected:
    virtual void read(const aiScene *scene) {
        //Read materials and textures.
        for(unsigned int cm=0;cm<scene->mNumMaterials;cm++) {
            this->materials.emplace_back(scene->mMaterials[cm]);
        }

        //Read vertices, normals, texture coordinates, and material type
        for(unsigned int cm=0;cm<scene->mNumMeshes;cm++) {
            this->meshes.emplace_back();

            const aiMesh *mesh = scene->mMeshes[cm];

            meshes[cm].materialId=mesh->mMaterialIndex;

            meshes[cm].vertices.resize(mesh->mNumVertices);
            meshes[cm].normals.resize(mesh->mNumVertices);
            meshes[cm].textureCoords.resize(mesh->mNumVertices);

            for(unsigned int cv=0;cv<mesh->mNumVertices;cv++) {
                meshes[cm].vertices[cv]=mesh->mVertices[cv];
                meshes[cm].normals[cv]=mesh->mNormals[cv];
                if(this->materials[meshes[cm].materialId].texture()) {
                    meshes[cm].textureCoords[cv].x=mesh->mTextureCoords[0][cv].x;
                    meshes[cm].textureCoords[cv].y=mesh->mTextureCoords[0][cv].y;
                }
            }
            meshes[cm].faces.resize(mesh->mNumFaces);
            for(unsigned int cf=0;cf<mesh->mNumFaces;cf++) {
                meshes[cm].faces[cf]=mesh->mFaces[cf];
            }
        }
    }
};

#endif	/* MODEL_HPP */