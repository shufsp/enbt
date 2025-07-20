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
	-i <input_file>			Input file with list of ips
	-t <csv|toml|json>		Specifies the type of input file
	-o <output_path>		Specifies the output. Default is 'servers.dat'
```

## Examples
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
Several formats are supported!
```
enbt -i servers -t toml
```
```
enbt -i servers -t json
``` 
```
enbt -i servers -t csv
``` 
