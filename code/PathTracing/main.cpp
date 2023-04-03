#include<iostream>
#include<vector>
#include"Model.h"
#include"Scene.h"
#include"Render.h"
#include"xmlLoader.h"

int main(int argc, char** argv) {
	
	string type = argv[1];
	int spp = atoi(argv[2]);
	cout << "using model: " << type << " SPP: " << spp << endl;
	//string type = "staircase";
	//string type = "cornell-box";
	//string type = "veach-mis";
	string modelpath = "../model/" + type + "/" + type + ".obj";
	string xmlpath = "../model/" + type + "/" + type + ".xml";
	string imagepath_png = "../result/" + type + ".png";
	string imagepath_exr = "../result/" + type + ".exr";
	std::unique_ptr<Model> model = make_unique<Model>(modelpath);
	Scene scene;
	xmlLoader xmlFileLoader;
	xml xmlContent;
	xmlContent = xmlFileLoader.readXML(xmlpath);
	
	
	//set light
	model->SetLight(xmlContent);
	//set camera
	scene.SetCamera(xmlContent.cameraPos, xmlContent.cameraUp, xmlContent.cameraFront, xmlContent.fov);
	//build BVH tree
	scene.Add(std::move(model));
	scene.BuildBVH();

	//render image
	Render renderer(xmlContent.width, xmlContent.height, spp);
	renderer.Rendering(scene);
	renderer.SaveExr(imagepath_exr);
	renderer.SavePNG(imagepath_png);
}