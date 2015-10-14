#ifndef SKELETAL_ANIMATION_MODEL_HPP
#define	SKELETAL_ANIMATION_MODEL_HPP

#include "model.hpp"

#include <unordered_map>
#include <functional>

//Create animation frames of 3D models with skeletal animations imported using AssImp (http://assimp.sourceforge.net/)
//Only tested with COLLADA files
//Supports (for simplicity): vertices, normals, textures, and skeleton animations
//Bone-related class structure:
//SkeletalAnimationModel (derived from Model)
//  0-many Bone (transformation)
//  0-many MeshExtended (derived from Mesh)
//    0-many BoneWeights (offsetMatrix and vertex weights)
//      1 boneId (position in Bone-vector)
//  0-many Animation (duration, ticksPerSecond)
//    0-many Channel (positions, rotations, and scales)
//      1 boneId (position in Bone-vector)

//For AssImp versions < 3.1 (I think). Will wait a year a so before I use the 3.1 aiMatrix4x4-constructor instead
aiMatrix4x4 aiMatrix4x4Compose(const aiVector3D& scaling, const aiQuaternion& rotation, const aiVector3D& position) {
    aiMatrix4x4 r;

    aiMatrix3x3 m = rotation.GetMatrix();

    r.a1 = m.a1 * scaling.x;
    r.a2 = m.a2 * scaling.x;
    r.a3 = m.a3 * scaling.x;
    r.a4 = position.x;

    r.b1 = m.b1 * scaling.y;
    r.b2 = m.b2 * scaling.y;
    r.b3 = m.b3 * scaling.y;
    r.b4 = position.y;

    r.c1 = m.c1 * scaling.z;
    r.c2 = m.c2 * scaling.z;
    r.c3 = m.c3 * scaling.z;
    r.c4= position.z;

    r.d1 = 0.0;
    r.d2 = 0.0;
    r.d3 = 0.0;
    r.d4 = 1.0;

    return r;
}

class Bone {
public:
    aiMatrix4x4 transformation;

    unsigned int parentBoneId;
    bool hasParentBoneId;

    Bone(): hasParentBoneId(false) {}
};

class BoneWeights {
public:
    aiMatrix4x4 offsetMatrix;
    std::vector<aiVertexWeight> weights;

    unsigned int boneId;
};

class MeshExtended : public Mesh {
public:
    std::vector<BoneWeights> boneWeights;
};

class Animation {
public:
    class Channel {
    public:
        unsigned int boneId;

        std::vector<aiVectorKey> positions;
        std::vector<aiQuatKey> rotations;
        std::vector<aiVectorKey> scales;
    };

    double duration;
    double ticksPerSecond;

    std::vector<Channel> channels;

private:
    aiVector3D interpolateFunction(const aiVector3D& beforeMatrix, const aiVector3D& afterMatrix, double factor) const {
        aiVector3D diffMatrix=afterMatrix-beforeMatrix;
        diffMatrix*=factor;

        return beforeMatrix+diffMatrix;
    }
    aiQuaternion interpolateFunction(const aiQuaternion& beforeMatrix, const aiQuaternion& afterMatrix, double factor) const {
        aiQuaternion returnMatrix;
        aiQuaternion::Interpolate(returnMatrix, beforeMatrix, afterMatrix, factor);

        return returnMatrix;
    }
public:
    //time in seconds
    template<class KeyType>
    auto interpolate(const std::vector<KeyType>& keys, double time, bool loop=true) const -> decltype(keys.begin()->mValue) {
        time*=ticksPerSecond;

        if(loop) {
            time=fmod(time, duration);
        }
        else if(time>=duration) {
            return keys[keys.size()-1].mValue;
        }

        unsigned int keyBefore=0, keyAfter=0;
        double frameDuration=1.0, frameTime=0.0;
        for(unsigned int cv=0;cv<keys.size();cv++) {
            if(time<keys[cv].mTime) {
                keyAfter=cv;
                if(cv==0) {
                    keyBefore=keys.size()-1;
                    frameDuration=keys[0].mTime;
                    frameTime=time;
                }
                else {
                    keyBefore=cv-1;
                    frameDuration=keys[keyAfter].mTime-keys[keyBefore].mTime;
                    frameTime=time-keys[keyBefore].mTime;
                }
                break;
            }
        }

        double factor=frameTime/frameDuration;

        return interpolateFunction(keys[keyBefore].mValue, keys[keyAfter].mValue, factor);
    }
};

template<class MaterialType=Material, class MeshType=MeshExtended>
class SkeletalAnimationModel : public Model<MaterialType, MeshType> {
public:
    class MeshFrame {
    public:
        std::vector<aiVector3D> vertices;
        std::vector<aiVector3D> normals;

        const MeshType& mesh;
        
        MeshFrame(const MeshType& mesh): vertices(mesh.vertices.size()), normals(mesh.normals.size()), mesh(mesh) {}
    };
    
private:
    //Add bone if it does not yet exist, and return boneId
    unsigned int getBoneId(const aiNode* node) {
        unsigned int boneId;
        if(boneName2boneId.count(node->mName.data)==0) {
            bones.emplace_back();
            boneId=bones.size()-1;
            boneName2boneId[node->mName.data]=boneId;

            bones[boneId].transformation=node->mTransformation;
        }
        else {
            boneId=boneName2boneId[node->mName.data];
        }

        return boneId;
    }

public:
    std::vector<Animation> animations;
    std::vector<Bone> bones;
    std::unordered_map<std::string, unsigned int> boneName2boneId;

    //Updates the transformation matrices for the bones that are part of the animation channels.
    //Which bones get their transformation matrices updated can be found in animations[animationId].channels[].boneId
    void createFrame(unsigned int animationId, double time, bool loop=true) {
        if(animationId<animations.size()) {
            for(auto& channel: animations[animationId].channels) { 
                aiVector3D scale=animations[animationId].interpolate(channel.scales, time, loop);
                aiQuaternion rotation=animations[animationId].interpolate(channel.rotations, time, loop);
                aiVector3D position=animations[animationId].interpolate(channel.positions, time, loop);
                bones[channel.boneId].transformation=aiMatrix4x4Compose(scale, rotation, position);
            }
        }
    }

    //Receives the frame vertices and normals for the given mesh.
    //Run after SkeletalAnimationModel::createFrame.
    MeshFrame getMeshFrame(const MeshType& mesh) const {
        MeshFrame meshFrame(mesh);

        //Calculate new frame vertices and normals
        for(unsigned int cb=0;cb<mesh.boneWeights.size();cb++) {
            const auto& boneWeights=mesh.boneWeights[cb];

            aiMatrix4x4 transformation;
            const std::function<void(unsigned int)> calculateBoneTransformation=[&](unsigned int boneId){
                if(bones[boneId].hasParentBoneId) {
                    calculateBoneTransformation(bones[boneId].parentBoneId);
                }
                transformation*=bones[boneId].transformation;
            };
            calculateBoneTransformation(boneWeights.boneId);
            transformation*=boneWeights.offsetMatrix;

            for(auto& weight: boneWeights.weights) {
                meshFrame.vertices[weight.mVertexId]+=weight.mWeight*(transformation*mesh.vertices[weight.mVertexId]);
                meshFrame.normals[weight.mVertexId]+=weight.mWeight*(aiMatrix3x3(transformation)*mesh.normals[weight.mVertexId]);
            }
        }

        return meshFrame;
    }

    //Draws the given mesh frame.
    //Currently only supports 1 diffuse texture per material
    virtual void drawMeshFrame(const MeshFrame& meshFrame) const {
        const MeshType& mesh=meshFrame.mesh;
        
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
            glNormal3f(meshFrame.normals[face.mIndices[0]].x, meshFrame.normals[face.mIndices[0]].y, meshFrame.normals[face.mIndices[0]].z);
            if(texture)
                glTexCoord2f(mesh.textureCoords[face.mIndices[0]].x, mesh.textureCoords[face.mIndices[0]].y);
            glVertex3f(meshFrame.vertices[face.mIndices[0]].x, meshFrame.vertices[face.mIndices[0]].y, meshFrame.vertices[face.mIndices[0]].z);

            glNormal3f(meshFrame.normals[face.mIndices[1]].x, meshFrame.normals[face.mIndices[1]].y, meshFrame.normals[face.mIndices[1]].z);
            if(texture)
                glTexCoord2f(mesh.textureCoords[face.mIndices[1]].x, mesh.textureCoords[face.mIndices[1]].y);                    
            glVertex3f(meshFrame.vertices[face.mIndices[1]].x, meshFrame.vertices[face.mIndices[1]].y, meshFrame.vertices[face.mIndices[1]].z);

            glNormal3f(meshFrame.normals[face.mIndices[2]].x, meshFrame.normals[face.mIndices[2]].y, meshFrame.normals[face.mIndices[2]].z);
            if(texture)
                glTexCoord2f(mesh.textureCoords[face.mIndices[2]].x, mesh.textureCoords[face.mIndices[2]].y);
            glVertex3f(meshFrame.vertices[face.mIndices[2]].x, meshFrame.vertices[face.mIndices[2]].y, meshFrame.vertices[face.mIndices[2]].z);
        }
        glEnd();
    }

    //TODO: draw using newer OpenGL with skinning on GPU instead of CPU
    //This function would then replace getMeshFrame and drawMeshFrame(const MeshFrame&)
    //Also, when finished, drawFrame should use this function as follows:
    //createFrame(animationId, time);
    //for(auto& mesh: this->meshes)
    //    drawMeshFrame(mesh);
    virtual void drawMeshFrame(const MeshType& mesh) const {
        throw std::logic_error("Not implemented");
    }
    
    //Convenient function to draw a frame directly without using createFrame, getMeshFrame, and drawMeshFrame separately. 
    void drawFrame(unsigned int animationId, double time) {
        createFrame(animationId, time);
        for(auto& mesh: this->meshes) {
            MeshFrame meshFrame=getMeshFrame(mesh);
            drawMeshFrame(meshFrame);
        }
    }

    //Read the 3D model
    virtual void read(const std::string& filename, unsigned int assimpImporterFlags=aiProcessPreset_TargetRealtime_Fast) {
        Assimp::Importer importer;

        const aiScene *scene = importer.ReadFile(filename, assimpImporterFlags);

        if(scene) {
            Model<MaterialType, MeshType>::read(scene);
            read(scene);
        }
    }

protected:
    virtual void read(const aiScene *scene) {
        //Find channels, and the bones used in the channels
        for(unsigned int ca=0;ca<scene->mNumAnimations;ca++) {
            animations.emplace_back();
            auto& animation=animations[ca];

            animation.duration=scene->mAnimations[ca]->mDuration;
            animation.ticksPerSecond=scene->mAnimations[ca]->mTicksPerSecond;

            animation.channels.resize(scene->mAnimations[ca]->mNumChannels);
            for(unsigned int cc=0;cc<scene->mAnimations[ca]->mNumChannels;cc++) {
                animation.channels[cc].positions.resize(scene->mAnimations[ca]->mChannels[cc]->mNumPositionKeys);
                animation.channels[cc].scales.resize(scene->mAnimations[ca]->mChannels[cc]->mNumScalingKeys);
                animation.channels[cc].rotations.resize(scene->mAnimations[ca]->mChannels[cc]->mNumRotationKeys);

                for(unsigned int cp=0;cp<scene->mAnimations[ca]->mChannels[cc]->mNumPositionKeys;cp++) {
                    animation.channels[cc].positions[cp]=scene->mAnimations[ca]->mChannels[cc]->mPositionKeys[cp];
                }
                for(unsigned int cs=0;cs<scene->mAnimations[ca]->mChannels[cc]->mNumScalingKeys;cs++) {
                    animation.channels[cc].scales[cs]=scene->mAnimations[ca]->mChannels[cc]->mScalingKeys[cs];
                }
                for(unsigned int cr=0;cr<scene->mAnimations[ca]->mChannels[cc]->mNumRotationKeys;cr++) {
                    animation.channels[cc].rotations[cr]=scene->mAnimations[ca]->mChannels[cc]->mRotationKeys[cr];
                }

                const aiNode* node=scene->mRootNode->FindNode(scene->mAnimations[ca]->mChannels[cc]->mNodeName);
                animations[ca].channels[cc].boneId=getBoneId(node);
            }
        }

        //Find all the bones, and their parent bones, connected to the meshes
        for(unsigned int cm=0;cm<scene->mNumMeshes;cm++) {
            for(unsigned int cb=0;cb<scene->mMeshes[cm]->mNumBones;cb++) {
                const aiNode* node=scene->mRootNode->FindNode(scene->mMeshes[cm]->mBones[cb]->mName);                    
                this->meshes[cm].boneWeights.emplace_back();
                unsigned int boneId=getBoneId(node);
                this->meshes[cm].boneWeights[cb].boneId=boneId;
                this->meshes[cm].boneWeights[cb].offsetMatrix=scene->mMeshes[cm]->mBones[cb]->mOffsetMatrix;

                for(unsigned int cw=0;cw<scene->mMeshes[cm]->mBones[cb]->mNumWeights;cw++) {
                    this->meshes[cm].boneWeights[cb].weights.emplace_back(scene->mMeshes[cm]->mBones[cb]->mWeights[cw]);
                }

                if(!bones[boneId].hasParentBoneId) {
                    //Populate Bone::parentBoneIds
                    node=node->mParent;
                    while(node!=scene->mRootNode) {
                        unsigned int parentBoneId=getBoneId(node);
                        bones[boneId].parentBoneId=parentBoneId;
                        bones[boneId].hasParentBoneId=true;
                        boneId=parentBoneId;

                        node=node->mParent;
                    }
                }
            }
        }
    }
};

#endif	/* SKELETAL_ANIMATION_MODEL_HPP */
