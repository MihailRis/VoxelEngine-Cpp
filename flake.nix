{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system: {
        devShells.default = with nixpkgs.legacyPackages.${system}; mkShell {
          nativeBuildInputs = [ cmake pkg-config ];
          buildInputs = [ glm glfw glew zlib libpng libvorbis openal luajit curl ]; # libglvnd
          packages = [ glfw mesa freeglut entt ];
          LD_LIBRARY_PATH = "${wayland}/lib:$LD_LIBRARY_PATH";
        };
    });
}
