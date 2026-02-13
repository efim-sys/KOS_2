#include <Arduino.h>
#include <vector>
#include "SD_MMC.h"
#include "USBCDC.h"
#include "3D.h"
#include "KOS/KOS.h"
#include "KUI/KUI.h"

String path;

float scale = 100;

struct {
    float x = 0;
    float y = 0;
    float z = 0;
    float rx, ry, rz;
} PoV;

struct Vertex {
    float x;
    float y;
    float z;
};

struct Face {
    Vertex  v[3];
    Vertex  normal;
    Vertex  center;
};

Vertex rotation = {0, 0, 0};

std::vector<Vertex> vertices;
std::vector<Vertex> normals;
std::vector<Face>   faces;

Vertex rotateVertex(Vertex in) {
    Vertex out = {
        in.x*cosf(rotation.z) - in.y*sinf(rotation.z),
        in.x*sinf(rotation.z) + in.y*cosf(rotation.z),
        in.z
    };
    return out;
}

float dot(Vertex a, Vertex b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vertex center(Face a) {
    return Vertex {
        (a.v[0].x + a.v[1].x + a.v[2].x) / 3.0f,
        (a.v[0].y + a.v[1].y + a.v[2].y) / 3.0f,
        (a.v[0].z + a.v[1].z + a.v[2].z) / 3.0f,
    };
}

void parseFile() {
    File obj = SD_MMC.open(path);  // Open File for reading

    while(obj.available()) {       // While file is not empty

        String str = obj.readStringUntil('\n');   // Read one line from file

        std::vector<String> values = {""};        // Create vector of strings

        // We will hold every parameter splitted by space. example: {"v", "0.432", "2.334", "-1.411"}

        for(int i = 0; i < str.length(); i++) {
            if(str[i] == ' ' or str[i] == '/') {
                values.back().trim();
                values.push_back("");
            }
            else values.back()+=str[i];
            
        }

        if(values[0] == "v") {     // If string holds vertex data
            vertices.push_back(Vertex{values[1].toFloat(), values[2].toFloat(), values[3].toFloat()});
        }
        else if (values[0] == "vn") {    // If string holds normal data
            normals.push_back(Vertex{values[1].toFloat(), values[2].toFloat(), values[3].toFloat()});
        }
        else if (values[0] == "f") {     // If string holds face data
            faces.push_back(Face{
                {
                    vertices[values[1].toInt()-1],
                    vertices[values[4].toInt()-1],
                    vertices[values[7].toInt()-1],
                },
                normals[values[9].toInt()-1]
            });
            faces.back().center = center(faces.back());
        }
    }

    obj.close();

    USBSerial.println("Faces:");

    for(Face i : faces) {
        for(auto j : i.v) {
            USBSerial.printf("Vertex: %f/%f/%f\n", j.x, j.y, j.z);
        }
        USBSerial.printf("Normal: %f/%f/%f\n\n", i.normal.x, i.normal.y, i.normal.z);
    }

}

float distance(Vertex a, Vertex b) {
    Vertex d = {
        a.x-b.x,
        a.y-b.y,
        a.z-b.z
    };
    return sqrtf(d.x*d.x + d.y*d.y + d.z*d.z);
}



bool sortAlgorithm(Face a, Face b) {
    return distance(a.center, Vertex{15, 0, 0}) < distance(b.center, Vertex{15, 0, 0});
}

void renderScene() {
    canvas.clear();
    std::vector<Face> f(faces.size());

    for(uint i = 0; i < f.size(); i++) {
        f[i] = {
            {
                rotateVertex(faces[i].v[0]),
                rotateVertex(faces[i].v[1]),
                rotateVertex(faces[i].v[2])
            },
            rotateVertex(faces[i].normal),
            rotateVertex(faces[i].center)
        };
    }

    std::sort(f.begin(), f.end(), sortAlgorithm);

    for(Face fc : f) {
        // if(fc.normal.x >= 0) continue;
        
        float lighting = dot(fc.normal, Vertex{-1, -1, 1}) / 1.732050808f;

        lighting = max(lighting, 0.0f);
        lighting = 100.0f + lighting*150.0f;

        lgfx::rgb888_t color;
        color.set(lighting, lighting, lighting);

        canvas.fillTriangle(
            120 + fc.v[0].y * scale, 
            120 - fc.v[0].z * scale, 
            120 + fc.v[1].y * scale, 
            120 - fc.v[1].z * scale, 
            120 + fc.v[2].y * scale, 
            120 - fc.v[2].z * scale, 
            color);
    }

    canvas.pushSprite(0, 0);
}

void showObj2(void * arg) {
    KUI::terminateWindow(false);
    parseFile();

    while(true) {
        renderScene();
        vTaskDelay(3);
        rotation.z += PI/100;
    }
}

void viewer3D_init(String _path) {
    path = _path;
    xTaskCreatePinnedToCore(showObj2, "showObj2", 8192, NULL, 4, NULL, 1);
}