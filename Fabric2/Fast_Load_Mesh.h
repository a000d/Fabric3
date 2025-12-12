#ifndef FAST_LOAD_MESH_H
#define FAST_LOAD_MESH_H

#include <Qt3DExtras/Qt3DWindow>
#include <QtWidgets/QHBoxLayout>
#include <QMainWindow>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
// #include <iostream>
#include <Qt3DCore/QEntity>
#include <QGeometryRenderer>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QAttribute>
#include <Qt3DRender/QCamera>
#include <QPhongMaterial>
#include <QOrbitCameraController>
#include "utils.hpp""

class Fast_Load_Mesh{
    Qt3DCore::QEntity* root_entity;
    Qt3DCore::QGeometry* geometry;
    Qt3DCore::QEntity* mesh_entity;
    Qt3DExtras::QPhongMaterial *material;

public:

    void Clear(){
        mesh_entity->setParent(static_cast<Qt3DCore::QNode*>(nullptr));
        mesh_entity->deleteLater();
    }
    void Set_Color(int x, int y, int z){

        material->setAmbient(QColor(x, y, z));
    }

    Fast_Load_Mesh(Qt3DCore::QEntity* root_entity,
                   vector<v3_f>& vertices,
                   vector<v3<uint>>& indices,
                   vector<v3_f>& normals){


        this->root_entity = root_entity;

        geometry = new Qt3DCore::QGeometry(root_entity);
        mesh_entity = new Qt3DCore::QEntity(root_entity);


        //------------------------------

        // QVector<QVector3D> vertices;
        // QVector<uint> indices;

        // vertices = {
        //    {-1.0f, -1.0f, 0.0f}, // 0: 左下
        //    { 1.0f, -1.0f, 0.0f}, // 1: 右下
        //    { 1.0f,  1.0f, 0.0f}, // 2: 右上
        //    {-1.0f,  1.0f, 0.0f}  // 3: 左上
        // };


        // indices = {
        //     {0, 2, 1},  // 第一个三角形
        //     {3, 2, 0}   // 第二个三角形
        // };

        // normals = {
        //     {0.0f, 0.0f, 1.0f}, // 0: 指向正Z轴
        //     {0.0f, 0.0f, 1.0f}, // 1: 指向正Z轴
        //     {0.0f, 0.0f, 1.0f}, // 2: 指向正Z轴
        //     {0.0f, 0.0f, 1.0f}  // 3: 指向正Z轴
        // };


        auto *vertexBuffer = new Qt3DCore::QBuffer(geometry);
        vertexBuffer->setData(QByteArray(
            reinterpret_cast<const char*>(vertices.data()),
            vertices.size() * sizeof(QVector3D)
            ));

        auto *indexBuffer = new Qt3DCore::QBuffer(geometry);
        indexBuffer->setData(QByteArray(
            reinterpret_cast<const char*>(indices.data()),
            indices.size() * sizeof(unsigned int)*3
            ));

        auto *normalBuffer = new Qt3DCore::QBuffer(geometry);
        normalBuffer->setData(QByteArray(
            reinterpret_cast<const char*>(normals.data()),
            normals.size() * sizeof(QVector3D)
            ));


        // 7. 创建顶点属性
        auto *positionAttribute = new Qt3DCore::QAttribute(geometry);
        positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
        positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        positionAttribute->setBuffer(vertexBuffer);
        positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setByteOffset(0);
        positionAttribute->setByteStride(sizeof(QVector3D));
        positionAttribute->setCount(vertices.size()); // 注意：这里是顶点数，不是索引数


        // 10. 创建法线属性 (引用法线缓冲区)
        auto *normalAttribute = new Qt3DCore::QAttribute(geometry);
        normalAttribute->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
        normalAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        normalAttribute->setBuffer(normalBuffer);  // 使用独立的法线缓冲区
        normalAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
        normalAttribute->setVertexSize(3);  // nx, ny, nz
        normalAttribute->setByteOffset(0);  // 从缓冲区开始位置读取
        normalAttribute->setByteStride(0);  // 重要：数据连续排列，stride为0
        normalAttribute->setCount(normals.size());


        // 8. 创建索引属性
        auto *indexAttribute = new Qt3DCore::QAttribute(geometry);
        indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
        indexAttribute->setBuffer(indexBuffer);
        indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
        indexAttribute->setVertexSize(1); // 索引是标量
        indexAttribute->setByteOffset(0);
        indexAttribute->setByteStride(sizeof(unsigned int));
        indexAttribute->setCount(indices.size()*3); // 索引数量

        geometry->addAttribute(positionAttribute);
        geometry->addAttribute(indexAttribute);
        geometry->addAttribute(normalAttribute);

        Qt3DRender::QGeometryRenderer* renderer  = new Qt3DRender::QGeometryRenderer(root_entity);
        renderer ->setGeometry(geometry);
        renderer ->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
        renderer->setVertexCount(indices.size()*3);

        renderer->setIndexOffset(0);
        renderer->setFirstInstance(0);
        renderer->setFirstVertex(0);

        material = new Qt3DExtras::QPhongMaterial(mesh_entity);
        material->setAmbient(QColor(255, 0, 0)); // 红色


        mesh_entity->addComponent(renderer);
        mesh_entity->addComponent(material);

        mesh_entity->setParent(root_entity);

    }




};















#endif // FAST_LOAD_MESH_H
