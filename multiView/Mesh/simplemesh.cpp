#include "simplemesh.h"

void SimpleMesh::update(){
    recomputeNormals();
}

void SimpleMesh::clear(){
    vertices.clear();
    triangles.clear();
    verticesNormals.clear();
}

void SimpleMesh::recomputeNormals () {
    computeVerticesNormals();
}

Vec3Df SimpleMesh::computeTriangleNormal(unsigned int id ){
    const Triangle &t = triangles[id];
    Vec3Df normal = Vec3Df::crossProduct(vertices[t.getVertex (1)] - vertices[t.getVertex (0)], vertices[t.getVertex (2)]- vertices[t.getVertex (0)]);
    normal.normalize();
    return normal;
}

void SimpleMesh::computeVerticesNormals(){
    verticesNormals.clear();
    verticesNormals.resize( vertices.size() , Vec3Df(0.,0.,0.) );

    for( unsigned int t = 0 ; t<triangles.size(); ++t ){
        Vec3Df const &tri_normal = computeTriangleNormal(t);
        verticesNormals[ triangles[t].getVertex(0) ] += tri_normal;
        verticesNormals[ triangles[t].getVertex(1) ] += tri_normal;
        verticesNormals[ triangles[t].getVertex(2) ] += tri_normal;
    }

    for( unsigned int v = 0 ; v < verticesNormals.size() ; ++v ) verticesNormals[ v ].normalize();
}

// Access and colour each individual vertex here
void SimpleMesh::glTriangle(unsigned int i){
    const Triangle &t = triangles[i];

    for(unsigned int j = 0 ; j < 3 ; j++ ){
        glNormal(verticesNormals[t.getVertex(j)]*normalDirection);
        glVertex(vertices[t.getVertex(j)]);
    }
}

void SimpleMesh::draw()
{
    glLineWidth(.5);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH);

    glBegin (GL_TRIANGLES);

    glColor3f(0.25, 0.25, 0.5);
    for(unsigned int i=0; i<triangles.size(); i++) glTriangle(i);

    glEnd();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH);
}
