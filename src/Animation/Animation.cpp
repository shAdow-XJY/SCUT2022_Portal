#include "Animation.h"
#include <iostream>
#include <glut.h>
#include <LoadModel/stb_image.h>
void Animation::init()
{
    this->scene = aiImportFile("../../src/Animation/models/man.fbx", aiProcessPreset_TargetRealtime_MaxQuality);
    animations.insert(pair<string, const aiScene*>("idle", aiImportFile("../../src/Animation/models/idle.fbx", aiProcessPreset_TargetRealtime_MaxQuality)));
    initAssetAnimaion();
    if (scene == nullptr)
    {
        cout << "Could not read model file for Model." << endl;
        //exit(1);
    }
    else {
        cout << "load fbx success" << endl;
    }
    

    initial_state = std::vector<FBXMesh>();
    // save initial state of the mesh so that mesh transformations can be applied.
    for (int i = 0; i < scene->mNumMeshes; i++)
    {
        FBXMesh newMesh = FBXMesh(std::vector<aiVector3D>(), std::vector<aiVector3D>());
        aiMesh* mesh = scene->mMeshes[i];
        for (int j = 0; j < mesh->mNumVertices; j++)
        {
            newMesh.vertices.push_back(mesh->mVertices[j]);
            newMesh.normals.push_back(mesh->mNormals[j]);
        }
        initial_state.push_back(newMesh);
    }
}


void Animation::initAssetAnimaion()
{
    string aniName[] = {"walk","run","jump","dying","turnRight","turnLeft","turnBack"};

    for (string name : aniName) {
        string baseUrl = "../../src/Animation/models/" + name + ".fbx";
        const aiScene*  result = aiImportFile(baseUrl.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
        if (result == nullptr)
        {
            cout << "Could not read animation file for Model." << endl;
            //exit(1);
        }
        else {
            cout << "load fbx success" << endl;
        }
        animations.insert(pair<string, const aiScene*>(name, aiImportFile(baseUrl.c_str(), aiProcessPreset_TargetRealtime_MaxQuality)));
    }
}

string Animation::getCurrentAnimation()
{
    return this->current_animation;
}

void Animation::update(int millisSinceStart, bool autoDisplay)
{
    aiAnimation* anim = animations[current_animation]->mAnimations[0];
    double tick = fmod((millisSinceStart * anim->mTicksPerSecond) / 1000.0, anim->mDuration);   
    updating(anim, tick);
}

void Animation::updating(aiAnimation* anim, double tick)
{
    for (unsigned int i = 0; i < anim->mNumChannels; i++)
    {
        aiNodeAnim* node = anim->mChannels[i];

        aiMatrix4x4 rotationMatrix = get_interpolated_rotation(tick, node);
        aiMatrix4x4 positionMatrix = get_interpolated_position(tick, node);
        // we assume the only node with multiple position keyframes is the root node
        if (node->mNumPositionKeys > 1)
        {
            positionMatrix = aiMatrix4x4();
            aiVector3D vec1(1.0f);
            vec1 *= positionMatrix;
            rootPosition = vec1;
        }

        aiNode* skeletonNode = scene->mRootNode->FindNode(node->mNodeName);

        skeletonNode->mTransformation = positionMatrix * rotationMatrix;
    }

    for (unsigned int idx_mesh = 0; idx_mesh < scene->mNumMeshes; idx_mesh++)
    {
        aiMesh* mesh = scene->mMeshes[idx_mesh];
        for (unsigned int idx_vert = 0; idx_vert < mesh->mNumVertices; idx_vert++)
        {
            mesh->mVertices[idx_vert] = aiVector3D(0);
            mesh->mNormals[idx_vert] = aiVector3D(0);
        }

        for (unsigned int idx_bone = 0; idx_bone < mesh->mNumBones; idx_bone++)
        {
            aiBone* bone = mesh->mBones[idx_bone];
            aiNode* node = scene->mRootNode->FindNode(bone->mName);
            aiMatrix4x4 boneTransform = bone->mOffsetMatrix;
            while (node != nullptr)
            {
                boneTransform = node->mTransformation * boneTransform;
                node = node->mParent;
            }

            aiMatrix4x4 boneTransformTranspose = boneTransform;
            boneTransformTranspose.Transpose();

            for (unsigned int idx_weight = 0; idx_weight < bone->mNumWeights; idx_weight++)
            {
                aiVertexWeight weight = bone->mWeights[idx_weight];
                mesh->mVertices[weight.mVertexId] = 1.0f * (boneTransform * initial_state[idx_mesh].vertices[weight.mVertexId]);
                mesh->mNormals[weight.mVertexId] = 1.0f * (boneTransformTranspose * initial_state[idx_mesh].normals[weight.mVertexId]);
            }
        }
    }

}

void Animation::display()
{
    aiNode* root = this->scene->mRootNode;
    renderDisplay(this->scene, this->scene->mRootNode, map<int, int>());
}

void Animation::setAnimation(string animationName)
{
    current_animation = animationName;
}

void Animation::cleanup()
{
}