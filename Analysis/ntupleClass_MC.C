#define ntupleClass_MC_cxx
#define NCUTS 8
#define NPARTICLES 560
#define NMU 3
#define mumass 0.1056583715
#define PhiMass 1.019461 // Phi mass in GeV
#define OmegaMass 0.78265 // Omega mass in GeV
#define ptmin 2.0

#include "ntupleClass_MC.h"
#include "Utilities.C"
#include "PdgId_list.C"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

int Idsummary2D[NCUTS][NPARTICLES][NPARTICLES] = {0};
int Idsummary2D_Gen[NPARTICLES][NPARTICLES] = {0};

// ######################################### MC: SIGNAL ANALYSIS CUTFLOW

// Cuts: (over triplets)
// * cut[1] -> Chi2 triplet vertex (in 0 - 15)
// * cut[2] -> There are (2 mu glb w/ pt>ptmin (=2) e 1 mu Tracker con pt>ptminTrack (=0.5)) & |eta|<2.4
// * cut[3] -> Triplet mass (in 1.62 - 2 GeV)
// * cut[4] -> The 3 possible pairs of mu of the triplet have proper DeltaR (<0.8)
// * cut[5] -> The 3 possible pairs of mu of the triplet have proper |DeltaZ| (<0.5)
// * cut[6] -> Cut on the dimuon mass w.r.t. Phi(1020) per pairs of mu of the triplet w/ opposite sign
// * cut[7] -> Cut on the dimuon mass w.r.t. Omega(782) per pairs of mu of the triplet w/ opposite sign

void ntupleClass_MC::LoopMC_New(TString datasetName){
    
    if (fChain == 0) return;
    Long64_t nentries = fChain->GetEntriesFast();
    // Variables definition
    int ntripl, trInd = 0, ind = 0, mu_Ind[NMU] = {0}, mu[NMU] = {0}, muGen[NMU] = {0}, NgoodTripl = 0, NbadTripl = 0, cut[NCUTS] = {0}, cutevt[NCUTS] = {0}, triplIndex[1000] = {0}, Ncut = 0, IdsummaryDaughter[NCUTS][NPARTICLES] = {0}, IdsummaryMother[NCUTS][NPARTICLES] = {0}, IdsummaryDaughter_Gen[NPARTICLES] = {0}, IdsummaryMother_Gen[NPARTICLES] = {0};
    float ptminTrack = 0.5, DeltaRmax = 0.8, DeltaZmax = 0.5, DeltaZ1 = 0, DeltaZ2 = 0, DeltaZ3 = 0, ntripltot = 0;
    double dimu[NMU] = {0}, massmin = 1.62, massmax = 2.00, sigmaPhi = 0.011, sigmaOmega = 0.0085, TripletVtx_Chi2max = 15, EtaMax = 2.4;
    TString listCut[NCUTS], pId[NPARTICLES];
    // Variables for the final tree
    double Pmu3 = 0, cLP = 0, segmComp = 0, fv_nC = 0, fv_dphi3D = 0, fv_d3Dsig = 0, d0sig = 0, mindca_iso = 0, trkRel = 0;
    float tKink = 0;
    //Variables inizialization
    Fill_particleName(pId);
    cutevt[0] = nentries;
    Fill_CutName(listCut);
    
    gStyle->SetOptStat(1111);
    // Creation of output file & final tree
    TString foutName = "AnalysedTree/AnalysedTree_MC_";
    if (strcmp(datasetName, "Ds") == 0)
        foutName += "DsTau3mu.root";
    if (strcmp(datasetName, "B0") == 0)
        foutName += "B0Tau3mu2.root";
    if (strcmp(datasetName, "Bp") == 0)
        foutName += "BpTau3mu2.root";
    if (strcmp(datasetName, "MiniBias") == 0)
        foutName += "MiniBias2.root";
    TFile *fout = new TFile(foutName, "RECREATE");
    fout->cd();
    TTree *tree = new TTree("FinalTree","FinalTree");
    TreeFin_Init(tree, evt, Pmu3, cLP, tKink, segmComp, fv_nC, fv_dphi3D, fv_d3Dsig, d0sig, mindca_iso, trkRel);
    // Creation of histograms for variables BEFORE cuts
    TDirectory *dirBeforeCuts = fout->mkdir("BeforeCuts");
    dirBeforeCuts->cd();
    TH1D *hMass_tripl_BC, *hChi2Vertex, *hMass_di_Zero_BC, *hMass_di_Zero2_BC, *hPtRes_BC, *hPtRes_BC_mu[NMU], *hPtResBarrel_BC, *hPtResBarrel_BC_mu[NMU], *hPtResEndcap_BC, *hPtResEndcap_BC_mu[NMU]; TH2D *hMassvsChi2;
    TH1F *hMass_quad_BC, *hMass_quad_Zero_BC;
    TH1I *hPdgId_Gen, *hMotherPdgId_Gen; TH2I *hPdgId2D_Gen;
    TCanvas *PdgIdCanvas_Gen, *PdgIdMotherCanvas_Gen, *PdgIdCanvas2D_Gen;
    InitHistoBC(hMass_tripl_BC, hChi2Vertex, hMassvsChi2, hMass_quad_BC, hMass_quad_Zero_BC, hMass_di_Zero_BC, hMass_di_Zero2_BC, hPtRes_BC, hPtRes_BC_mu, hPtResBarrel_BC, hPtResBarrel_BC_mu, hPtResEndcap_BC, hPtResEndcap_BC_mu, hPdgId_Gen, hMotherPdgId_Gen, hPdgId2D_Gen);
    fout->cd();
    // Creation of histograms for variables AFTER cuts
    TDirectory *dirAfterCuts = fout->mkdir("AfterCuts");
    dirAfterCuts->cd();
    TH1I *hNtripl; TH1F *hChi2Track, *hmassQuad, *hmassQuad_Zero;
    TH1D *hMassTriRes, *hMassTriResBarrel, *hMassTriResEndcap, *hmassdi, *hPtRes_AC, *hPtRes_AC_mu[NMU], *hPtResBarrel_AC, *hPtResBarrel_AC_mu[NMU], *hPtResEndcap_AC, *hPtResEndcap_AC_mu[NMU], *hNMatchedStat, *hFlightDist, *hFlightDist_Signif, *hPtErrOverPt, *hPt_tripl_good, *hPt_tripl_fake, *hDeltaX, *hDeltaY, *hDeltaZ, *hDeltaX_fake, *hDeltaY_fake, *hDeltaZ_fake; TH2D *hFlightDistvsP;
    InitHistoAC(hNtripl, hChi2Track, hMassTriRes, hMassTriResBarrel, hMassTriResEndcap, hmassdi, hmassQuad, hmassQuad_Zero, hPtRes_AC, hPtRes_AC_mu, hPtResBarrel_AC, hPtResBarrel_AC_mu, hPtResEndcap_AC, hPtResEndcap_AC_mu, hNMatchedStat, hFlightDist, hFlightDist_Signif, hFlightDistvsP, hPtErrOverPt, hPt_tripl_good, hPt_tripl_fake, hDeltaX, hDeltaY, hDeltaZ, hDeltaX_fake, hDeltaY_fake, hDeltaZ_fake);
    fout->cd();
    // Creation of histo StepByStep
    TDirectory *dirStepByStep = fout->mkdir("StepByStep");
    dirStepByStep->cd();
    TDirectory *dirSingleMu = dirStepByStep->mkdir("SingleMu"); // Single mu variables histo
    dirSingleMu->cd();
    TH1D *hPt[NCUTS], *hPt_mu[NCUTS][NMU], *hEta[NCUTS], *hEta_mu[NCUTS][NMU], *hPhi[NCUTS], *hVx[NCUTS], *hVy[NCUTS], *hVz[NCUTS];
    InitHistoStepByStep_SingleMu(hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz);
    dirStepByStep->cd();
    TDirectory *dirTriplet = dirStepByStep->mkdir("Triplet");  // Triplet variables histo
    dirTriplet->cd();
    TH1D *hPt_tripl[NCUTS], *hEta_tripl[NCUTS], *hPhi_tripl[NCUTS], *hMass_tripl[NCUTS];
    InitHistoStepByStep_Triplet(hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl);
    dirStepByStep->cd();
    TDirectory *dirPdgId = dirStepByStep->mkdir("PdgId"); // PdgId histo
    dirPdgId->cd();
    TH1I *hPdgId_cut[NCUTS], *hMotherPdgId_cut[NCUTS]; TH2I *hPdgId2D_cut[NCUTS];
    TCanvas *PdgIdCanvas_cut[NCUTS], *PdgIdMotherCanvas_cut[NCUTS], *PdgIdCanvas2D_cut[NCUTS];
    InitHistoStepByStep_PdgId(hPdgId_cut, hMotherPdgId_cut, hPdgId2D_cut);
    dirStepByStep->cd();
    fout->cd();
    // Creation of total histograms
    TH1I *hCutEff = new TH1I("CutEff_Ntriplets", "CutEff_Ntriplets", NCUTS, 0.5, (NCUTS+0.5));
    TH1I *hCutEffEvt = new TH1I("CutEff_NEvents", "CutEff_NEvents", NCUTS, 0.5, (NCUTS+0.5));
    
    //Loop over the events
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        ntripl = 0, trInd = 0; int cutevt2[NCUTS] = {0};
        Long64_t ientry = fChain->LoadTree(jentry);
        fChain->GetEntry(ientry);
        //Check number of tracks in the primary vertex
        if(PV_NTracks > NMU){
            //Loop over the TRIPLETS
            for (int j=0; j<TripletVtx_Chi2->size(); j++){
                ntripltot++; Ncut = 0;
                // BEFORE cuts
                //Matching between index of single mu of the triplet (mu#_Ind) & that of 'MUONID' (mu#)
                MatchIndex("ID", j, mu_Ind, mu);
                // Fill histograms
                FillHistoBC("MC", j, hMass_tripl_BC, hChi2Vertex, hMassvsChi2, hMass_quad_BC, hMass_quad_Zero_BC, hMass_di_Zero_BC, hMass_di_Zero2_BC, hPtRes_BC, hPtRes_BC_mu, hPtResBarrel_BC, hPtResBarrel_BC_mu, hPtResEndcap_BC, hPtResEndcap_BC_mu, IdsummaryDaughter_Gen, IdsummaryMother_Gen, Idsummary2D_Gen);
                FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                //CUT 1 : check condition on * Chi2 vertex ( 0 < Chi2 < 15)
                if (TripletVtx_Chi2->at(j) < TripletVtx_Chi2max && TripletVtx_Chi2->at(j) > 0 ){
                    Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                    FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                    // CUT 2 :
                    // Check that mu1 is glb & pt>ptmax & |eta|<Etamax
                    if((Muon_isGlobal->at(mu[0]) == 1) && (MuonPt->at(mu[0]) > ptmin) && abs(Mu1_Eta->at(mu_Ind[0])) < EtaMax){
                        DeltaZ1 = Muon_vz->at(mu[0]);
                        // Check that mu2 is glb & pt>ptmax & |eta|<Etamax
                        if((Muon_isGlobal->at(mu[1]) == 1) && (MuonPt->at(mu[1]) > ptmin) && abs(Mu2_Eta->at(mu_Ind[1])) < EtaMax){
                            DeltaZ2 = Muon_vz->at(mu[1]);
                            // Check that mu3 is tracker & pt>0.5 & |eta|<Etamax
                            if((Muon_isTrackerMuon->at(mu[2]) == 1) && (MuonPt->at(mu[2]) > ptminTrack) && abs(Mu3_Eta->at(mu_Ind[2])) < EtaMax){
                                DeltaZ3 = Muon_vz->at(mu[2]);
                                Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                //CUT 3: check condition on trimuon mass
                                if(Triplet_Mass->at(j) > massmin && Triplet_Mass->at(j) < massmax){
                                    Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                    FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                    //CUT 4: Loop on PAIRS of muons of the triplet & check DeltaR
                                    if(isPairDeltaRGood(j, DeltaRmax) == true){
                                        Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                        FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                        //CUT 5 : Check |Delta Z|
                                        if(isPairDeltaZGood(DeltaZ1, DeltaZ2, DeltaZ3, DeltaZmax) == true){
                                            Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                            FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                            //CUT 6: VETO on Phi(1020) mass
                                            Fill_DimuonMass(mu_Ind, mu, dimu);
                                            if(isPairNotAPhi(dimu, sigmaPhi) == true){
                                                Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                                FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                                //CUT 7: VETO on Omega(782) mass
                                                if(isPairNotAOmega(dimu, sigmaOmega) == true){
                                                    Ncut++; ntripl++; triplIndex[trInd] = j; trInd++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } // end loop on triplets
        }
        // N. events that passed each selection
        for (int k=1; k<NCUTS; k++){
            if(cutevt2[k] > 0) cutevt[k]++;
        }
        // Histo N. triplets passed for each event
        hNtripl->Fill(ntripl);
        if(ntripl > 0) {
            cutevt[NCUTS-1]++; cut[NCUTS-1]++;
            ind = BestTripletFinder(triplIndex, ntripl);
            //RiMatching between index of single mu of the triplet (mu#_Ind) & that of  'MUONID' (mu#) & Ricomputing the 3 possible dimuon masses
            MatchIndex("ID", ind, mu_Ind, mu);
            TreeFin_Fill(tree, ind, mu_Ind, mu, Pmu3, cLP, tKink, segmComp, fv_nC, fv_dphi3D, fv_d3Dsig, d0sig, mindca_iso, trkRel);
            Fill_DimuonMass(mu_Ind, mu, dimu);
            FillHistoStepByStep("MC", ind, mu_Ind, mu, NCUTS-1, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
            // Resolution & final histograms
            MatchIndex("Gen", ind, mu_Ind, muGen);
            if(muGen[0] != -999 && muGen[1] != -999 && muGen[2] != -999) {
                NgoodTripl++;
                FillHistoResoPt_AC(muGen, hPtRes_AC, hPtRes_AC_mu, hPtResBarrel_AC, hPtResBarrel_AC_mu, hPtResEndcap_AC, hPtResEndcap_AC_mu);
                StudyOnTriplet("good", ind, mu, hDeltaX, hDeltaY, hDeltaZ, hPt_tripl_good);
                FillHistoResoTriplMass(mu_Ind, mu, hMassTriRes, hMassTriResBarrel, hMassTriResEndcap);
            }
            if(muGen[0] == -999 && muGen[1] == -999 && muGen[2] == -999) {
                NbadTripl++;
                StudyOnTriplet("bad", ind, mu, hDeltaX_fake, hDeltaY_fake, hDeltaZ_fake, hPt_tripl_fake);
            }
            FillHistoAC(ind, mu, hChi2Track, hNMatchedStat, hFlightDist, hFlightDist_Signif, hFlightDistvsP, hPtErrOverPt, hmassdi, dimu, hmassQuad, hmassQuad_Zero);
        }
        
        if (ientry < 0) break;
    }//end loop on events
    cut[0] = ntripltot;
    //Print general info
    cout << endl;
    cout << "N. EVENTS -> " << nentries << endl << endl;
    cout << "N. TRIPLETS -> " << ntripltot << endl << endl;
    cout << "Triplets survived: " << cutevt[NCUTS-1] << " || Good: " << NgoodTripl << " , Bad: " << NbadTripl << endl;
    //Histo of cuts Efficiency
    TCanvas *canvEvt = new TCanvas("CutEfficiency_Nevents", "CutEfficiency_Nevents", 0, 0, 1200, 1000);
    Draw_CutEffCanvas(canvEvt, hCutEffEvt, cutevt, listCut);
    TCanvas *canv = new TCanvas("CutEfficiency_Ntriplets", "CutEfficiency_Ntriplets", 0, 0, 1200, 1000);
    Draw_CutEffCanvas(canv, hCutEff, cut, listCut);
    //PdgId histos
    dirStepByStep->cd(); dirPdgId->cd();
    Draw_PdgIdCanvas_StepByStep(PdgIdCanvas_cut, hPdgId_cut, IdsummaryDaughter, PdgIdMotherCanvas_cut, hMotherPdgId_cut, IdsummaryMother, PdgIdCanvas2D_cut, hPdgId2D_cut, Idsummary2D, pId);
    dirStepByStep->cd(); fout->cd();
    //PdgId histos Gen BC
    dirBeforeCuts->cd();
    Draw_PdgIdCanvasGen(PdgIdCanvas_Gen, hPdgId_Gen, IdsummaryDaughter_Gen, PdgIdMotherCanvas_Gen, hMotherPdgId_Gen, IdsummaryMother_Gen, PdgIdCanvas2D_Gen, hPdgId2D_Gen, Idsummary2D_Gen, pId);
    fout->cd();
    //Write and close the file
    fout->Write();
    fout->Close();
}

// #########################################
