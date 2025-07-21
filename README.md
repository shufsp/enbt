# enbt
A fast and simple tool for creating Minecraft server lists from ip lists of various file formats or terminal stdin via pipe.


## Build
Clone the repo then run the following
```sh
mkdir build && cd build
cmake ..
make
```


## Usage
```
Usage: ./enbt -i <ip_list_input> [options]
Options
        -i <input_file>                 Input file with list of ips
        -t <csv|toml|json>              Specifies the type of input file
        -o <output_path>                Specifies the output. Default is 'servers.dat'
        --stdout                        Outputs the servers nbt to stdout. Equivalent to -o stdout
```

### Examples
Generate a servers.dat file from a list of ips in csv format
```
enbt -i servers_list.csv
```
Generate with custom output location
```
enbt -o "some_path/.minecraft/servers.dat" -i servers.toml
```
```
enbt -o "custom_name.dat" -i ips.json
```
If the file doesn't have an extension, you can provide the `-t` option.
```
enbt -i servers -t toml
```
Generate servers.dat with piped input
```
echo "{\"servers\": [{\"ip\": \"1.0.0.1\", \"name\": \"Piped server\", \"icon\": \"/9j/4AAQS\", \"accept_textures\": true}]}" | enbt -t json
```
```
echo "Server1,/9j/4AAQSkZJRgABAQIAJQAl,153.74.117.133,1" | enbt -t csv
```
Convert input to servers.dat nbt and output directly to terminal
```
$ echo "Server1,/9j/4AAQSkZJRgABAQIAJQAl,153.74.117.133,1" | enbt -t csv --stdout

        servers
nameServericon/9j/4AAQSkZJRgABAQIAJQAip153.74.117.133acceptTextures
```
```
echo "Server1,/9j/4AAQSkZJRgABAQIAJQAl,153.74.117.133,1" | enbt -t csv --stdout >> servers.out
```
## Input Format
Here are some examples for how you should format your toml, csv, and json to pass into enbt.
The properties [according to minecraft wiki](https://minecraft.wiki/w/Servers.dat_format) are:

- icon			: Base64-encoded PNG data of the server icon.
- ip 			: The IP address of the server.
- name 		 	: The name of the server as defined by the player.
- acceptTextures 	: 1 or 0 (true/false) - 0 if the player has selected Never when prompted to install a server resource pack.

The base64 encoded strings below are shortened for example purposes.
If a server entry is missing a required property, it will be skipped and not outputted into the resulting servers.dat file.

### CSV
```csv
Server1,/9j/4AAQSkZJRgABAQIAJQAl,153.74.117.133,1
Server2,/9j/4AAQSkZJRgABAQIAJQAl,114.171.20.137,0
Server3,/9j/4AAQSkZJRgABAQIAJQAl,118.59.75.213,1
Server4,/9j/4AAQSkZJRgABAQIAJQAl,123.59.131.207,1
Server5,/9j/4AAQSkZJRgABAQIAJQAl,223.138.205.100,1
Server6,/9j/4AAQSkZJRgABAQIAJQAl,111.102.151.232,1
```
### JSON
```json
{
  "servers": [
    {
      "icon": "/9j/4AAQSkZJRgABAQIAJQAl",
      "ip": "192.168.1.1",
      "name": "Server One Json",
      "accept_textures": true
    },
    {
      "icon": "/9j/4AAQSkZJRgABAQIAJQAl",
      "ip": "192.168.1.2",
      "name": "Server Two Json",
      "accept_textures": false
    }
  ]
}
```
### TOML
```toml
[[servers]]
icon = "/9j/4AAQSkZJRgABAQIAJQAl"
ip = "192.168.1.1"
name = "Server One Toml"
accept_textures = true

[[servers]]
icon = "/9j/4AAQSkZJRgABAQIAJQAl"
ip = "192.168.1.2"
name = "Server Two Toml"
accept_textures = false

[[servers]]
icon = "/9j/4AAQSkZJRgABAQIAJQAl"
ip = "192.168.1.3"
name = "Server Three Toml"
accept_textures = true
```
