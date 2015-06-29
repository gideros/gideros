cd ..
cd plugins

for d in *; do
cd $d/source
if [ -f $d.pro ]; then
$QT/bin/qmake "CONFIG+=warn_off" $d.pro
make clean > /dev/null 2>&1
make
fi
cd ..
cd ..
done

cd ..
cd scripts

