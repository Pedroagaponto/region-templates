if [ $# -ne 3 ] 
then
  echo "Usage: `basename $0 $1 $2` {dakota-file}{original-img-path}{mask-img-path}"
  exit $E_BADARGS
fi

FILE=$1
IMG_PATH_ORIG=$2
IMG_PATH_MASK=$3

sed -i 's/T2/T2 integer/g' ${FILE}
sed -i 's/G1/G1 integer\n                      2.4000000000e+02 green integer\n                      2.3400000000e+02 red integer\n                      2.2200000000e+02 blue integer\n                      5.5000000000e+00 T1 float/g' ${FILE}
sed -i 's/G2/G2 integer/g' ${FILE}
sed -i 's/\<minSize\>/minSize integer/g' ${FILE}
sed -i 's/\<maxSize\>/maxSize integer/g' ${FILE}
sed -i 's/minSizePl/minSizePl integer/g' ${FILE}
sed -i 's/minSizeSeg/minSizeSeg integer\n                      1.0200000000e+03 maxSizeSeg integer\n                      8.0000000000e+00 fillHolesConnectivity integer\n                      4.0000000000e+00 watershedConnectivity integer\n                      [0.26235,0.0514831,0.0114217] target_std floatarray\n                      [-0.632356,-0.0516004,0.0376543] target_mean floatarray/g' ${FILE}
sed -i "s#recon#reconConnectivity integer\n                      "$IMG_PATH_ORIG" input_img rt\n                      "$IMG_PATH_MASK" input_ref_img rt#g" ${FILE}


# 5.8386293768e+00 T2
#                       4.3640994725e+01 G1
#                       1.3333330468e+01 G2
#                       3.0460877144e+00 minSize
#                       1.4924662404e+03 maxSize
#                       7.1132759171e+01 minSizePl
#                       3.9332878668e+01 minSizeSeg
#                       7.9202353265e+00 recon


# 2.2200000000e+02 blue integer
#                       1.0200000000e+03 maxSizeSeg integer
#                       8.0000000000e+00 fillHolesConnectivity integer
#                       4.0000000000e+00 watershedConnectivity integer
#                       [0.26235,0.0514831,0.0114217] target_std floatarray
#                       [-0.632356,-0.0516004,0.0376543] target_mean floatarray
#                       /home/willian/Desktop/images15/image10.tiff input_img rt
#                       /home/willian/Desktop/images15/image10.mask.png input_ref_img rt