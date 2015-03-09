cd ..
cd plugins

for d in *; do
cd $d/source
if [ -f $d.pro ]; then
$QT/bin/qmake $d.pro
make clean
make
fi
cd ..
cd ..
done

cd ..
cd scripts

