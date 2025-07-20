#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "NBTWriter.h"
#include "toml.hpp"
#include "nlohmann/json.hpp"
#include <vector>

namespace fs = std::filesystem;

struct nbtserver {
	std::string icon; // base64
	std::string ip;
	std::string name;
	bool accept_textures;
};

void usage(const std::string_view program) {
	std::cout << "Usage: " << program << " -i <ip_list_input> [options]\n";
	std::cout << "Options\n";
	std::cout << "\t-i <input_file>\t\t\tInput file with list of ips\n";
	std::cout << "\t-t <csv|toml|json>\t\tSpecifies the type of input file\n";
	std::cout << "\t-o <output_path>\t\tSpecifies the output. Default is 'servers.dat'\n";
}

void parse_arg(const std::string_view cmd, 
	       std::string& stored_value,
	       const std::string_view default_value, 
	       int* argc, 
	       char*** argv,
	       bool arg_required=false) {
	if (arg_required && (*argc) < 2) {
		std::cout << '\'' << cmd << "' requires an argument\n";
		exit(1);
	}
	if (stored_value == default_value)
		stored_value = (*argv)[1];
	(*argv)++;
	(*argc)--;
}

std::vector<nbtserver> parse_servers_json(const std::string& content) {
	using json = nlohmann::json;
	if (content.empty()) {
		std::cout << "json file content is empty. no servers.dat created\n";
		exit(1);
	}
	
	if (!json::accept(content)) {
		// TODO show where its malformed/show error from nlohmann json?
		std::cout << "json is malformed. validate the syntax and try again\n";
		exit(1);
	}

	json config = json::parse(content);
	if (!config.contains("servers") || !config["servers"].is_array()) {
		std::cout << "json is malformed. requires a 'servers' array\n";
		exit(1);
	}

	std::vector<nbtserver> servers{};
    	servers.reserve(config["servers"].size());

	for (const auto& server : config["servers"]) {
		if (!server.contains("icon") || !server.contains("ip") || 
			!server.contains("name") || !server.contains("accept_textures")) {
			std::cout << "warning: a server entry is missing required fields. it will not be added to the servers list\n";
			continue;
		}

		servers.emplace_back(nbtserver{
			.icon = server.at("icon").get<std::string>(),
			.ip = server.at("ip").get<std::string>(),
			.name = server.at("name").get<std::string>(),
			.accept_textures = server.at("accept_textures").get<bool>()
		});
	}

	return servers;
}

std::vector<nbtserver> parse_servers_toml(const std::string& content) {
	if (content.empty()) {
		std::cout << "toml file content is empty. no servers.dat created\n";
		exit(1);
	}

  	const auto maybe_parsed = toml::try_parse_str(content.data());
	if (!maybe_parsed.is_ok()) {
		// TODO show error source from try_parse?
		std::cout << "toml file is malformed. validate the syntax and try again\n"; 
		exit(1);
	}

	auto config = maybe_parsed.unwrap();
	if (!config.at("servers").is_array_of_tables()) {
		std::cout << "toml is malformed. requires a 'servers' table as an array of tables\n";
		exit(1);
	}

	auto& server_tables = config["servers"].as_array();
	std::vector<nbtserver> servers{};
	servers.reserve(server_tables.size());

	for (const auto& server : server_tables) {
		servers.emplace_back(nbtserver{
			.icon = toml::find<std::string>(server, "icon"),
			.ip = toml::find<std::string>(server, "ip"),
			.name = toml::find<std::string>(server, "name"),
			.accept_textures = toml::find<bool>(server, "accept_textures")
		});
	}
	
	return servers;
}

std::vector<nbtserver> parse_servers_csv(const std::string& content) {
	if (content.empty()) {
		std::cout << "csv file content is empty. no servers.dat created\n";
		exit(1);
	}
	
	const size_t server_count = std::count(content.begin(), content.end(), '\n');

	std::vector<nbtserver> servers{};
	servers.reserve(server_count);

	std::string line;
	std::istringstream stream{content};
	while (std::getline(stream, line)) {
		// get nbt properties for this server by splitting delimiter
		// Example: Server Name,base6409ujisdfskdf,127.0.0.1,0
		std::string items[4]{};
		for (std::size_t i = 0, pos = 0; pos < line.size(); ++i) {
			auto const next_pos = std::find_if(line.begin() + pos, line.end(), [](auto& c){return c == ','||c=='|'||c==';';}) - line.begin();
			items[i] = line.substr(pos, next_pos - pos);
			pos = next_pos == line.size() ? line.size() : next_pos + 1;
		}	
		servers.emplace_back(nbtserver{ 
			.icon = items[1], 
			.ip = items[2], 
			.name = items[0], 
			.accept_textures = items[3][0] == '1' 
		});
	}

	return servers;
}

void ips_to_dat(const std::string_view input_path, const std::string_view output_path, const std::string_view format) {
	if (input_path.empty()) {
		std::cout << "Input path is empty\n";
		exit(1);
	}
	if (output_path.empty()) {
		std::cout << "Output path is empty\n";
		exit(1);
	}

	std::ifstream ips_stream(input_path.data());
	if (!ips_stream.is_open()) {
		std::cout << "Unable to open input file for reading (" << input_path << ")\n";
		exit(1);
	}

	std::stringstream buffer;
	buffer << ips_stream.rdbuf();

	const std::string ips_content = buffer.str();
	const std::vector<nbtserver> servers = [&](){
		if (format == "csv")
			return parse_servers_csv(ips_content);
		else if (format == "toml")
			return parse_servers_toml(ips_content);
		else if (format == "json")
			return parse_servers_json(ips_content);
		return std::vector<nbtserver>{};
	}();

	if (servers.empty()) {
		std::cout << "There are no servers in your input file\n";
		exit(1);
	}

	NBT::NBTWriter writer(output_path.data());
	writer.writeListHead("servers", NBT::idCompound, servers.size());
	for (const nbtserver& server : servers) {	
		#if 0
		std::cout << server.name << '\n';
		std::cout << server.icon << '\n';
		std::cout << server.ip << '\n';
		std::cout << server.accept_textures << '\n';
		std::cout << "------------------\n"; 
		#endif
	 	writer.writeCompound("");
		writer.writeString("name", server.name.data());
		writer.writeString("icon", server.icon.data());
		writer.writeString("ip", server.ip.data());
		writer.writeByte("acceptTextures", server.accept_textures);
		writer.endCompound();
	}
	writer.endCompound();
	writer.close();
	ips_stream.close();
}

int main(int argc, char** argv) {
	// TODO take input from pipe i.e. program | enbt

	const std::string_view program = argv[0];
	if (argc < 2) {
		usage(program);
		exit(1);
	}
	argv++;
	argc--;

	std::string input_path{};
	std::string output_path = "servers.dat";	
	std::string input_type = "csv";
	bool explicit_extension = false;

	while (argc > 0) {
		const std::string_view cmd = argv[0];
		if (cmd == "-i") {
			parse_arg(cmd, input_path, "", &argc, &argv, true);	
		} else if (cmd == "-o") {
			parse_arg(cmd, output_path, "servers.dat", &argc, &argv, true);
		} else if (cmd == "-t") {
			parse_arg(cmd, input_type, "csv", &argc, &argv, true);
			explicit_extension = true;
		} else {		
			std::cout << "unknown option '" << cmd << "'\n";
			usage(program);
			exit(1);
		}
		argv++;
		argc--;
	}

	if (input_path.empty()) {
		usage(program);
		exit(1);
	}

	if (!fs::exists(input_path)) {
		std::cout << "Can't load '" << input_path << "': file doesn't exist\n";
		exit(1);
	}

	if (!explicit_extension) {
		// work how its expected to unless overriden with -t
		auto ext = fs::path(input_path).extension().string();
		if (ext.empty()) {
			std::cout << "The input file provided does not have an extension. Provide an explicit input type with the -t option\n";
			exit(1);
		}
		input_type = ext.erase(0, 1); //remove '.'
	}

	if (input_type != "csv" && input_type != "toml" && input_type != "json") {
		std::cout << "Invalid value for -t '" << input_type << "'\n";
		exit(1);
	}
	
	ips_to_dat(input_path, output_path, input_type);
	
	return 0;
}

