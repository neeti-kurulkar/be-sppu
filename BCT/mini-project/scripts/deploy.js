const hre = require("hardhat");

async function main() {
    const EVoting = await hre.ethers.getContractFactory("EVoting");
    const evoting = await EVoting.deploy();
    await evoting.deployed();
    console.log("EVoting deployed to:", evoting.address);
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});