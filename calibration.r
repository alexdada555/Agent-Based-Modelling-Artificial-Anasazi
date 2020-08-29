#Run Calibration Process

library (EasyABC)


mindeathAge = c("unif",20,35)
maxdeathAge = c("unif",30,50)
minFertileAge = c("unif",20,35)
maxFertileAge = c("unif",25,40)
fissionProb = c("unif",0.1,0.15)
MaizeFieldData1 = c("unif",0.7,1.2)
MaizeFieldData2 = c("unif",0,0.5)
anasazi_prior=list(mindeathAge , maxdeathAge, minFertileAge, maxFertileAge, fissionProb, MaizeFieldData1, MaizeFieldData2)

target_data <- read.csv(file='target_data.csv')
tolerance=0.2


#runs the calibration
ABC_sim<-ABC_rejection(model=binary_model("bash model.sh") , prior=anasazi_prior, nb_simul=10, prior_test="(X2>X1) && (X4>X3)", summary_stat_target=c(target_data), tol=tolerance, use_seed = FALSE, seed_count = 0, n_cluster=1, verbose=FALSE, progress_bar = FALSE)
#ABC_sim<-ABC_rejection(model=binary_model("bash model.sh") , prior=anasazi_prior, nb_simul=10, prior_test=NULL, summary_stat_target=NULL, tol=NULL, use_seed = FALSE, seed_count = 0, n_cluster=1, verbose=FALSE, progress_bar = FALSE)

