#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void loadModel(string path)) {
    // Create an instance of the Importer class
    Assimp::Importer importer;
    const aiScene * scene = importer.ReadFile(fileName, aiProcess_Triangulate
        | aiProcess_FlipUVs
        | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }
    directory = path.substr(0, path.find_last_of("/"));
    processNode(scene->mRootNode, scene);
}

void procesNode(aiNode * node, const aiScene * scene) {
    // Process each mesh located at the mesh node
    for(unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // Process each children node
    for(unsigned int i = 0; i < node->mNumChildren; ++i) {
        procesNode(node->mChildren[i], scene);
    }
}
