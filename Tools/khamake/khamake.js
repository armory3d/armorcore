let versions = process.version.substring(1).split('.');
if ((Number(versions[0]) < 8 || (Number(versions[0]) === 8 && Number(versions[1]) < 9)) && process.version !== 'v7.4.0' /* Kode Studio 17.9 */) {
	console.error('Requires Node.js version 8.9 or higher but found ' + process.version + '.');
	process.exit(1);
}

require('./out/khamake.js');
