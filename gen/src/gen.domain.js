const fs = require('fs')
const pluralize = require('pluralize')
const { writeApi, writeModel } = require('./gen.utils')

async function main() {

    const args = [...process.argv.slice(2)];
    const apiIndex = args.indexOf("api") + 1;

    if (args.length) {
        const file = args[0];
        const name = pluralize(file, 1);
        const path = __dirname + "/../../src/models/" + name + ".json";
        let data = fs.readFileSync(path) + "";
        if (data.length) {
            data = JSON.stringify({ ...JSON.parse(data), _id: { $oid: "" } });

            await writeModel({ name, data });
            if (apiIndex) {
                writeApi({ name });
            }
        }
    } else {
        console.log("No name specified");
    }
}

main();