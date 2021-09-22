const quicktype = require('quicktype')
const fs = require('fs')

const dir = __dirname + "/tmp";

function capitalize(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}

function getOutput(name) {
    const output = dir + "/../../../src/api/" + name;

    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }

    if (!fs.existsSync(output)) {
        fs.mkdirSync(output, { recursive: true });
    }
    return output;
}

async function writeModel({ name, data }) {
    const input = dir + "/" + name + ".json";
    const Name = capitalize(name);
    const output = getOutput(name);

    fs.writeFileSync(input, data);
    await quicktype.main([input, '-l', 'c++', '-o',
        (output + "/" + name + ".hpp"),
        '--quiet',
        '--code-format', 'with-struct',
        '--namespace', 'DB_' + Name,
        '--all-properties-optional',
        '--no-boost',
        '--msbuildPermissive not-permissive'
    ]);
    fs.unlinkSync(input);
    let file = fs.readFileSync((output + "/" + name + ".hpp")) + "";
    file = file.split('#include "json.hpp"').join('#include "../../base/libs/json/json.hpp"')
    fs.writeFileSync((output + "/" + name + ".hpp"), file);
}

function writeApi({ name }) {
    const Name = capitalize(name);
    const apiDir = __dirname + "/api";
    const output = getOutput(name);

    for (const apiName of fs.readdirSync(apiDir)) {
        let apiFile = fs.readFileSync(apiDir + "/" + apiName) + "";
        const apiPath = output + "/" + name + "." + apiName.split(".temp").join(".hpp");
        apiFile = apiFile.split("<l>").join(name).split("<u>").join(Name);
        fs.writeFileSync(apiPath, apiFile);
    }

    writeMain({ name, Name });
    writeEnums({ Name })
}

function writeMain({ name, Name }) {
    const mainPath = dir + "/../../../src/main.cpp";
    let main = fs.readFileSync(mainPath) + "";

    let tokens = main.split("server.start");
    tokens[0] = tokens[0] + "\tserver.resolve(new " + Name + "Resolver());\n";
    main = tokens.join("server.start");

    tokens = main.split("\nint main");
    tokens[0] = tokens[0] + `\n#include "api/${name}/${name}.resolver.hpp"`;
    main = tokens.join("\nint main");

    fs.writeFileSync(mainPath, main);
}

function writeEnums({ Name }) {
    const enumPath = dir + "/../../../src/base/common/enums.hpp";
    let enumFile = fs.readFileSync(enumPath) + ""
    let tokens = enumFile.split("    Case(User, \"User\");\n");
    tokens[0] += "    Case(" + Name + ", \"" + Name + "\");\n";
    enumFile = tokens.join("    Case(User, \"User\");\n");
    fs.writeFileSync(enumPath, enumFile);
}

module.exports = { writeApi, writeModel };