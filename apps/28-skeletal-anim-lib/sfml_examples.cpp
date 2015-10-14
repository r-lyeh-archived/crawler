#include <GL/glew.h>
#include "skeletal_animation_model.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <chrono>
#include <thread>
#include <iostream>
#include <functional>

//If Linux: must run XInitThreads() from X11/Xlib.h before starting threads in X11
#ifdef __linux
#include<X11/Xlib.h>
#define INIT_THREADS XInitThreads();
#else
#define INIT_THREADS
#endif

const std::string modelPath="models/";

//Class that defines how we are to handle materials, and especially textures, in the 3D model.
//Only diffuse textures stored in external files are handled in this example
class SFMLMaterial : Material {
public:
    std::vector<sf::Texture> diffuseTextures;

    SFMLMaterial(const aiMaterial* material) {
        for(unsigned int cdt=0;cdt<material->GetTextureCount(aiTextureType_DIFFUSE);cdt++) {
            aiString path;
            material->GetTexture(aiTextureType_DIFFUSE, cdt, &path); //Assumes external texture files
            sf::Image image;
            image.loadFromFile(modelPath+path.C_Str());
            //Need to vertically flip the textures read with SFML
            image.flipVertically();

            diffuseTextures.emplace_back();
            diffuseTextures[cdt].loadFromImage(image);
        }
    }
    
    void bindTexture(aiTextureType textureType, unsigned int textureId) const {
        sf::Texture::bind(&diffuseTextures[textureId]);
    }
    
    bool texture() const {
        return diffuseTextures.size()>0;
    }
};

//Example 1 - draws an unanimated model
class UnanimatedAstroBoy {
public:
    Model<SFMLMaterial> model;
    
    UnanimatedAstroBoy() {
        model.read(modelPath+"astroBoy_walk_Maya.dae");
    }
    
    void draw() {
        model.draw();
    }
};

//Example 2 - draws a skeletal animated model using SkeletalAnimationModel::drawFrame, which consists of:
//SkeletalAnimationModel::createFrame makes the animation frame given animationId and time, 
//SkeletalAnimationModel::getMeshFrame receives the frame vertices and normals for the given mesh, and
//SkeletalAnimationModel::drawMeshFrame draws the given mesh frame. Example 3 and 4 show why we have these 3 functions.
class AstroBoy {
public:
    SkeletalAnimationModel<SFMLMaterial> model; //template argument specifies how we are to handle textures
    
    AstroBoy() {
        model.read(modelPath+"astroBoy_walk_Maya.dae");
    }
    
    //Draw the animation frame given time in seconds
    void drawFrame(double time) {
        model.drawFrame(0, time); //this function equals the following lines:
        //model.createFrame(0, time); //first parameter selects which animation to use
        //for(auto& mesh: model.meshes) {
        //    auto meshFrame=model.getMeshFrame(mesh);
        //    model.drawMeshFrame(meshFrame);
        //}
    }
};

//Example 3 - moves a mesh after the mesh of an animation frame is created.
//Modification happens here between SkeletalAnimationModel::getMeshFrame and SkeletalAnimationModel::drawMeshFrame
class AstroBoyMovingGlasses {
public:
    SkeletalAnimationModel<SFMLMaterial> model;
    
    AstroBoyMovingGlasses(const SkeletalAnimationModel<SFMLMaterial>& model): model(model) {}
    
    //Draw the animation frame given time in seconds
    void drawFrame(double time) {
        model.createFrame(0, time);
        for(unsigned int cm=0;cm<model.meshes.size();cm++) {
            auto meshFrame=model.getMeshFrame(model.meshes[cm]);
            if(cm==1) {
                for(auto& vertex: meshFrame.vertices)
                    vertex.z+=4.0*(cos(time*10.0)+1.0);
            }
            model.drawMeshFrame(meshFrame);
        }
    }
};

//Example 4 - changing animation through direct manipulation of a bone transformation matrix
//Modification happens here between SkeletalAnimationModel::createFrame and SkeletalAnimationModel::getMeshFrame
class AstroBoyHeadBanging {
public:
    SkeletalAnimationModel<SFMLMaterial> model;
    
    AstroBoyHeadBanging(const SkeletalAnimationModel<SFMLMaterial>& model): model(model) {}
    
    //Draw the animation frame given time in seconds
    void drawFrame(double time) {
        model.createFrame(0, time);
        
        unsigned int boneId=model.boneName2boneId.at("head");
        aiVector3D oldScale;
        aiQuaternion oldRotation;
        aiVector3D oldPosition;
        model.bones[boneId].transformation.Decompose(oldScale, oldRotation, oldPosition);
        aiMatrix3x3 newRotation;
        aiMatrix3x3::Rotation(cos(time*10.0), aiVector3D(0, 0, 1), newRotation);
        model.bones[boneId].transformation=aiMatrix4x4Compose(oldScale, aiQuaternion(newRotation)*oldRotation, oldPosition);
        
        for(auto& mesh: model.meshes) {
            auto meshFrame=model.getMeshFrame(mesh);
            model.drawMeshFrame(meshFrame);
        }
    }
};
    
//Example 5 - print bone hierarchy. Format: boneId boneName
template <class TextureHandler>
void printBoneHierarchy(const SkeletalAnimationModel<TextureHandler>& model) {
    //Find bone children from Bone::parentBoneId
    std::vector<std::vector<unsigned int> > boneChildrenIds(model.bones.size());
    for(unsigned int cb=0;cb<model.bones.size();cb++) {
        if(model.bones[cb].hasParentBoneId) {
            boneChildrenIds[model.bones[cb].parentBoneId].push_back(cb);
        }
    }
    
    //map boneId to boneName
    std::vector<std::string> boneId2BoneName(model.bones.size());
    for(auto& p: model.boneName2boneId) {
        boneId2BoneName[p.second]=p.first;
    }
    
    const std::function<void(unsigned int, unsigned int)> recursiveFunction=[&](unsigned int boneId, unsigned int level) {
        for(unsigned int c=0;c<level;c++)
            std::cout << "  ";
        std::cout << boneId << " " << boneId2BoneName[boneId] << std::endl;
        for(unsigned int childBoneId: boneChildrenIds[boneId]) {
            recursiveFunction(childBoneId, level+1);
        }
    };
    for(unsigned int cb=0;cb<model.bones.size();cb++) {
        if(!model.bones[cb].hasParentBoneId) {
            recursiveFunction(cb, 0);
        }
    }
}

//Create window, handle events, and OpenGL draw-function
class SFMLApplication {
public:
    SFMLApplication(): contextSettings(32), 
            window(sf::VideoMode(800, 640), "Skeletal Animation Library", sf::Style::Default, contextSettings) {}

    void start() {
        glewExperimental = GL_TRUE;
        glewInit();

        window.setFramerateLimit(100);
        window.setVerticalSyncEnabled(true);

        //Deactivate OpenGL context of window, will reopen in draw()
        window.setActive(false);

        //Start draw in a separate thread
        std::thread drawThread(&SFMLApplication::draw, this);

        //Event thread (main thread)
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::KeyPressed) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                        window.close();
                    }
                }
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }
        }
        
        //Wait for draw_thread to also finish for clean exit
        drawThread.join();
    }
    
private:
    sf::ContextSettings contextSettings;
    sf::Window window;
    
    void draw() {
        //Activate the window's context
        window.setActive();

        //Various settings
        glClearColor(0.5, 0.5, 0.5, 0.0f);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glEnable(GL_TEXTURE_2D);
        
        //Lighting
        GLfloat light_color[] = {0.9, 0.9, 0.9, 1.f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, light_color);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        
        //Setup projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        GLfloat ratio = float(window.getSize().x) / window.getSize().y;
        glFrustum(-ratio, ratio, -1.0, 1.0, 1.0, 500.0);

        //Loading models
        UnanimatedAstroBoy unanimatedAstroBoy;
        AstroBoy astroBoy;
        printBoneHierarchy(astroBoy.model);
        AstroBoyMovingGlasses astroBoyMovingGlasses(astroBoy.model);
        AstroBoyHeadBanging astroBoyHeadBanging(astroBoy.model);
        
        //Store start time
        std::chrono::time_point<std::chrono::system_clock> startTime=std::chrono::system_clock::now();
        
        //The rendering loop
        glMatrixMode(GL_MODELVIEW);
        while (window.isOpen()) {
            std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - startTime;
            double time=elapsed_seconds.count();
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glLoadIdentity();

            glTranslatef(0.0f, -20.0f, -45.0f);

            glPushMatrix();
            glRotatef(-time*50.0, 0.0, 1.0, 0.0);
            glTranslatef(20.0, 0.0, 0.0);            
            unanimatedAstroBoy.draw();
            glPopMatrix();
            
            glPushMatrix();
            glRotatef(-time*50.0+90.0, 0.0, 1.0, 0.0);
            glTranslatef(20.0, 0.0, 0.0);            
            astroBoy.drawFrame(time);
            glPopMatrix();
            
            glPushMatrix();
            glRotatef(-time*50.0+180.0, 0.0, 1.0, 0.0);
            glTranslatef(20.0, 0.0, 0.0);            
            astroBoyMovingGlasses.drawFrame(time);
            glPopMatrix();
            
            glRotatef(-time*50.0+270.0, 0.0, 1.0, 0.0);
            glTranslatef(20.0, 0.0, 0.0);            
            astroBoyHeadBanging.drawFrame(time);
            
            //Swap buffer (show result)
            window.display();
        }
    }
};

int main() {
    INIT_THREADS
            
    SFMLApplication sfmlApplication;
    sfmlApplication.start();
    
    return 0;
}
