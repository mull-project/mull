#!/usr/bin/env python3

import json
import dataclasses
from dacite import from_dict
import io
import sys


@dataclasses.dataclass
class ConfigField:
    type: str
    name: str
    description: str


@dataclasses.dataclass
class ConfigEntry:
    type: str
    name: str
    description: str
    fields: list[ConfigField] = dataclasses.field(default_factory=list)


def load_raw_config(path: str):
    with open(path, "r") as f:
        return json.loads(f.read())


def dump_cpp(conf: ConfigEntry, stream):
    stream.write(f"\n// {conf.description}\n")
    stream.write(f"struct Generated{conf.name} {{\n")
    for f in conf.fields:
        stream.write(f"  // {f.description}\n")
        stream.write(f"  {f.type} {f.name}{{}};\n")
    stream.write("};\n\n")


def dump_yaml_mapping(conf: ConfigEntry, stream):
    stream.write(f"template <> struct llvm::yaml::MappingTraits<{conf.name}> {{\n")
    stream.write(f"  static void mapping(llvm::yaml::IO &io, {conf.name} &config) {{\n")
    for f in conf.fields:
        stream.write(f'    io.mapOptional("{f.name}", config.{f.name});\n')
    stream.write("  }\n")
    stream.write("};\n")


def generate_headers(configs: list[ConfigEntry], output_dir: str):
    for conf in configs:
        output_file = f"{output_dir}/Generated{conf.name}.h"
        out = io.StringIO()
        dump_cpp(conf, out)
        with open(output_file, "w") as f:
            f.write(out.getvalue())


def generate_yaml_mapping(configs: list[ConfigEntry], output_dir: str):
    output_file = f"{output_dir}/GeneratedYamlMapping.cpp"
    out = io.StringIO()
    for conf in configs:
        dump_yaml_mapping(conf, out)
    with open(output_file, "w") as f:
        f.write(out.getvalue())


config_source = sys.argv[1]
output_dir = sys.argv[2]

raw_config = load_raw_config(config_source)
configs = []
for conf in raw_config:
    conf = from_dict(data_class=ConfigEntry, data=conf)
    configs.append(conf)


generate_headers(configs, output_dir)
generate_yaml_mapping(configs, output_dir)
