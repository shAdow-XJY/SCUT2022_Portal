#include "Animation.h"
#include <iostream>
#include <glut.h>
void Animation::init()
{
    //"D:/Microsoft Visual Studio Projects/PhysX-Tutorial-master/PhysX_3.4/SCUT2022_Portal/src/Animation/models/Model2_FBX/mannequin.fbx"
    scene = aiImportFile("../../src/Animation/models/Model2_FBX/mannequin.fbx", aiProcessPreset_TargetRealtime_MaxQuality);
    animations.push_back(aiImportFile("../../src/Animation/models/Model2_FBX/walk.fbx", aiProcessPreset_TargetRealtime_MaxQuality));
    animations.push_back(aiImportFile("../../src/Animation/models/Model2_FBX/jump.fbx", aiProcessPreset_TargetRealtime_MaxQuality));
    animations.push_back(aiImportFile("../../src/Animation/models/Model2_FBX/run.fbx", aiProcessPreset_TargetRealtime_MaxQuality));

    if (scene == nullptr)
    {
        cout << "Could not read model file for Model." << endl;
        exit(1);
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

    yRotate = PxMat44::PxMat44(PxIdentity);
}

void Animation::update(int millisSinceStart)
{
    aiAnimation* anim = animations[current_animation]->mAnimations[0];

    double tick = fmod((millisSinceStart * anim->mTicksPerSecond) / 1000.0, anim->mDuration);
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
    //get_bounding_box(scene, &scene_min, &scene_max);
}

void Animation::display()
{
    //float pos[4] = { -400, 1500, 1300, 1 };
    //glLightfv(GL_LIGHT0, GL_POSITION, pos);
    /*glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);*/

    aiNode* root = this->scene->mRootNode;
    //gluLookAt(-0.5, 600.5, 1300.5, -0.5, 200, 1.5, 0, 1, 0);
    //render(this->scene, this->scene->mRootNode, std::map<int, int>());
    
    /*PxMat44 yRotate(PxQuat(-PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));*/
    
    PxMat44 modelMatrix(PxShapeExt::getGlobalPose(*attachedRole->getShape(), *attachedRole->getActor()));
    PxMat44 rotate(PxQuat(-PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));

   
    renderDisplay(this->scene, this->scene->mRootNode, std::map<int, int>(), modelMatrix * rotate * yRotate);

}

void Animation::keyboard(unsigned char key)
{
    if (key == ' ')
    {
        current_animation = (current_animation + 1) % animations.size();
    }
}

void Animation::cleanup()
{
}

void Animation::attachRole(Role& role) {
    //��Ҫ����任����д�����û������λ��
    this->attachedRole = &role;
}

void Animation::changeOrientation(const PxQuat& orientation)
{
    yRotate = PxMat44(orientation);
}
