
#run model from R

# rm props/model.props

# run input to props script
mpirun bin/props_create.exe


#echo "GOT TO HERE 1"

mpirun bin/main.exe props/config.props props/model.props

#echo "GOT TO HERE 2"


mpirun bin/output_calibration.exe

#echo "GOT TO HERE 3"
