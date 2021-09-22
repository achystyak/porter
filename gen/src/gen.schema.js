const fs = require('fs')
const pluralize = require('pluralize')
const { writeApi, writeModel } = require('./gen.utils')

async function main() {
    const data = await fetch("http://localhost:7000/schema");
    const body = await data.json();

    for (const key of Object.keys(body.result)) {
        const data = JSON.stringify(body.result[key]);
        const name = pluralize(key, 1);

        if (name == 'auth' || name == 'user')
            continue;

        await writeModel({ name, data });
        writeApi({ name });

        console.log(key);
    }

    const mainPath = __dirname + "/../../src/main.cpp";
    let main = fs.readFileSync(mainPath) + "";
    main = main
        .split("int main").join("\nint main")
        .split("		").join("	")
        .split("server.start").join("\n    server.start");
    fs.writeFileSync(mainPath, main);
}

main();