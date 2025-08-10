namespace viewer3D {
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

    // void showObj(void * arg) {
        
    //     // vTaskDelay(100);

    //     KUI::terminateWindow(false);
    //     USBSerial.println(path);
        
    //     File obj = SD_MMC.open(path);

    //     // String type;

    //     std::vector<std::array<float, 3>> vertices;
    //     std::vector<std::array<float, 3>> normals;
    //     std::vector<std::array<uint32_t, 4>> faces;

    //     while(obj.available()) {
            
    //         String str = obj.readStringUntil('\n');
    //         std::vector<String> values = {""};
    //         for(int i = 0; i < str.length(); i++) {
    //             if(str[i] == ' ' or str[i] == '/') {
    //                 values.back().trim();
    //                 values.push_back("");
    //             }
    //             else values.back()+=str[i];
                
    //         }
    //         // USBSerial.print('{');
    //         // for(int i = 0 ; i < values.size(); i ++) {
    //         //     USBSerial.print('\"');
    //         //     USBSerial.print(values[i]);
    //         //     USBSerial.print('\"');
    //         //     if(i!=(values.size()-1)) USBSerial.print(", ");
    //         // }
    //         // USBSerial.println('}');

    //         if(values[0] == "v") {
    //             vertices.push_back(std::array<float, 3>{{values[1].toFloat(),values[2].toFloat(),values[3].toFloat()}});
    //         }
    //         else if(values[0] == "vn") {
    //             normals.push_back(std::array<float, 3>{{values[1].toFloat(),values[2].toFloat(),values[3].toFloat()}});
    //         }
    //         else if(values[0] == "f") {
    //             faces.push_back(std::array<uint32_t, 4>{{values[1].toInt()-1, values[4].toInt()-1, values[7].toInt()-1, values[9].toInt()-1}});
    //         }
    //     }

    //     // USBSerial.println("Done reading");
    //     //     str.trim();
    //     //     USBSerial.print("{");
    //     //     USBSerial.print(str);
    //     //     USBSerial.println("}");
    //     //     if(str.startsWith("v")) {
    //     //         USBSerial.println("Vertice found");
    //     //         vertices.push_back(std::array<float, 3>{{0,0,0}});
                
    //     //     }
    //     //     else if(str.startsWith("vn")) {
    //     //         USBSerial.println("Normal found");
    //     //         normals.push_back(std::array<float, 3>{{obj.parseFloat(), obj.parseFloat(), obj.parseFloat()}});
    //     //     }
    //     //     else if(str.startsWith("f")) {
    //     //         USBSerial.println("Face found");
    //     //         uint32_t val[9];
    //     //         for(int i = 0; i < 9; i++) {
    //     //             val[i] = obj.parseInt();
    //     //         }
    //     //         faces.push_back(std::array<uint32_t, 4>{{val[0]-1, val[3]-1, val[6]-1, val[8]-1}});
    //     //     }
            
    //     // }
    //     // // USBSerial.println("\nvertices:");
    //     // for(int i = 0; i < vertices.size(); i++) {
    //     //     // USBSerial.printf("%d %f, %f, %f\n", i, vertices[i][0], vertices[i][1], vertices[i][2]);
    //     // }

    //     // // USBSerial.println("\nNormals:");
    //     // for(int i = 0; i < normals.size(); i++) {
    //     //     // USBSerial.printf("%d %f, %f, %f\n", i, normals[i][0], normals[i][1], normals[i][2]);
    //     // }

    //     // // USBSerial.println("\nFaces:");
    //     // for(int i = 0; i < faces.size(); i++) {
    //     //     // USBSerial.printf("%d %d, %d, %d, %d\n", i, faces[i][0], faces[i][1], faces[i][2], faces[i][3]);
    //     // }

    //     float rotation = 0;

    //     // std::vector<>

    //     while(true) {
    //         canvas.clear();
    //         rotation += PI/100.0;

    //         // for(int a = 0; a < vertices.size(); a++) {
    //         //     float xa = vertices[a][0]*sinf(rotation) + vertices[a][1]*cosf(rotation);
    //         //     for(int b = 0; b < vertices.size(); b++) {
    //         //         float xb = vertices[b][0]*sinf(rotation) + vertices[b][1]*cosf(rotation);
    //         //         canvas.drawLine(120+xa*scale, 120+vertices[a][2]*scale, 120+xb*scale, 120+vertices[b][2]*scale, TFT_RED);
    //         //     }
    //         // }

    //         float norm[3];

            

    //         for(int i = 0; i < faces.size(); i++) {
    //             norm[0]=normals[faces[i][3]][0]*cosf(rotation+M_PI_2) - normals[faces[i][3]][1]*sinf(rotation+M_PI_2);
    //             norm[1]=normals[faces[i][3]][0]*sinf(rotation+M_PI_2) + normals[faces[i][3]][1]*cosf(rotation+M_PI_2);
    //             norm[2]=normals[faces[i][3]][2]*1;

    //             // if(i == 10) USBSerial.println(norm[1]);
    //             // if(i == 100) USBSerial.println(norm[1]);
    //             // if(i == 0) USBSerial.println(degrees(rotation));

    //             if (norm[1] >= 0.0) continue;

    //             float dot = norm[0]*1 +norm[1]*-1 +norm[2]*1;
    //             dot = (dot/1.732050808);                 // Divide by length of light vector {1; 1; 1}

    //             USBSerial.println(dot);
    //             if (dot < 0) dot = 0;
    //             if (dot > 1) dot = 1;

    //             float lighting = 100+dot*150;

    //             // if (lighting > 150) lighting = 150;

                

    //             int coords[6];

    //             for(int a = 0; a < 3; a++) {
    //                 int nA = faces[i][a];
    //                 int nB = faces[i][(a+1)%3];
    //                 float xa = vertices[nA][0]*sinf(rotation) + vertices[nA][1]*cosf(rotation);
    //                 float xb = vertices[nB][0]*sinf(rotation) + vertices[nB][1]*cosf(rotation);

    //                 coords[a*2] = 120+xa*scale;
    //                 coords[a*2+1] = 120-vertices[nA][2]*scale;
    //                 // canvas.drawLine(120+xa*scale, 120-vertices[nA][2]*scale, 120+xb*scale, 120-vertices[nB][2]*scale, TFT_RED);
    //             }

    //             lgfx::rgb888_t color;
    //             color.set(lighting, lighting, lighting);

    //             // USBSerial.printf("K=%d\n", i);

    //             canvas.fillTriangle(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], color);
    //         }

    //         canvas.pushSprite(0, 0);
    //     }

    //     vTaskDelete(NULL);
    // }

    void init(String _path) {
        path = _path;
        xTaskCreatePinnedToCore(viewer3D::showObj2, "showObj2", 8192, NULL, 4, NULL, 1);
    }
}