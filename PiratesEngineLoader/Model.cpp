#include "Model.h"

Model::Model(std::string filepath)
{
	std::string value;
	std::ifstream file(filepath, std::ios::app);
	if (file) {
		file.seekg(0, std::ios::end);
		value.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(value.data(), value.size());
		file.close();
	}throw(errno);

	Json = nlohmann::json::parse(value);
	Model::file = filepath;
	data = getdata();

}

void Model::Draw(Shader& shader)
{

}

std::vector<unsigned char> Model::getdata()
{
	std::string bytestext;
	std::string uri = Json["buffers"][0]["uri"];

	std::string filedirectory = file.substr(0, file.find_last_of('/') + 1);

	std::ifstream file((filedirectory + uri).c_str(), std::ios::app);
	if (file) {
		file.seekg(0, std::ios::end);
		bytestext.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(bytestext.data(), bytestext.size());
		file.close();
	}throw(errno);

	std::vector<unsigned char> data(bytestext.begin(), bytestext.end());

	return data;
}

std::vector<float> Model::GetFloats(nlohmann::json accessor)
{
	std::vector<float> vecfloat;

	unsigned int BufferViewInd = accessor.value("bufferview", 1);
	unsigned int count= accessor["count"];
	unsigned int accessorbyteoffset= accessor.value("byteoffset", 0);

	std::string type = accessor["type"];

	return std::vector<float>();
}

std::vector<float> Model::GetIndices(nlohmann::json accessor)
{

	return std::vector<float>();
}
