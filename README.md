# apov-tga-raw-converter
Converts targa raw format slices to the voxels.bin apov file entry.

To scan your meshes and generates the corresponding slices, you can use the
blender 2.9+ file available at the root of this repository.

Then copy paste the generated tga files into a folder (scans for example) at the
root of the repository, and run the following command to convert them into an
APoV voxels.bin file entry:
./bin/apov-tga-convert scans slice-count:256 read-id-before-dot
