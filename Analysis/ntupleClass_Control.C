#define ntupleClass_Control_cxx
#define NCUTS 8
#define NPARTICLES 560
#define NMU_C 2
#define NTOT 3
#define mumass 0.1056583715 // Muon mass in GeV
#define PhiMass 1.019461 // Phi mass in GeV
#define OmegaMass 0.78265 // Omega mass in GeV
#define ptmin 2.0

#include "ntupleClass_Control.h"
#include "Utilities_Control.C"
#include "PdgId_list_Control.C"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

// #########################################    DsPhiPi ANALYSIS CUTFLOW

// Cuts: (over triplets)
// * cut[1] -> Both mu have to be different from the track
// * cut[2] -> Chi2 triplet vertex (in 0 - 15)
// * cut[3] -> 2 mu w/ opposite charge
// * cut[4] -> Invariant dimuon mass in (1-1.04) GeV
// * cut[5] -> Longitudianl IP of the track w.r.t. the beam spot < 20 cm
// * cut[6] -> Transverse IP of the track w.r.t. the beam spot < 0.3 cm
// * cut[7] ->

void ntupleClass_Control::LoopControl(){
    if (fChain == 0) return;
    Long64_t nentries = fChain->GetEntriesFast();
    // Variables definition
    int ntripl, trInd = 0, ind = 0, mu_Ind[NTOT] = {0}, mu[NTOT] = {0}, muGen[NTOT] = {0}, NgoodTripl = 0, NbadTripl = 0, cut[NCUTS] = {0}, cutevt[NCUTS] = {0}, triplIndex[1000] = {0}, Ncut = 0, IdsummaryDaughter[NCUTS][NPARTICLES] = {0}, IdsummaryMother[NCUTS][NPARTICLES] = {0}, IdsummaryDaughter_Gen[NPARTICLES] = {0}, IdsummaryMother_Gen[NPARTICLES] = {0};
    float ptminTrack = 0.5, DeltaRmax = 0.8, DeltaZmax = 0.5, DeltaZ1 = 0, DeltaZ2 = 0, DeltaZ3 = 0, ntripltot = 0;
    double dimu[NTOT] = {0}, massmin = 1.62, massmax = 2.00, sigma = 0.011, TripletVtx_Chi2max = 15, EtaMax = 2.4;
    TString pId[NPARTICLES], listCut[NCUTS];
    //Variables inizialization
    cutevt[0] = nentries;
    Fill_particleName(pId);
    Fill_CutName(listCut);
    // Creation of the output file
    TFile *fout = new TFile("AnalysedTree/AnalysedTree_MC_DsPhiPi.root", "RECREATE");
    fout->cd();
    // Creation of histograms for variables BEFORE cuts
    TDirectory *dirBeforeCuts = fout->mkdir("BeforeCuts");
    dirBeforeCuts->cd();
    TH1D *hMass_tripl_BC, *hChi2Vertex, *hMass_di_Zero_BC, *hMass_di_Zero2_BC, *hPtRes_BC, *hPtRes_BC_mu[NMU_C], *hPtResBarrel_BC, *hPtResBarrel_BC_mu[NMU_C], *hPtResEndcap_BC, *hPtResEndcap_BC_mu[NMU_C]; TH2D *hMassvsChi2;
    TH1F *hMass_quad_BC, *hMass_quad_Zero_BC;
    TH1I *hPdgId_Gen, *hMotherPdgId_Gen; TH2I *hPdgId2D_Gen;
    TCanvas *PdgIdCanvas_Gen, *PdgIdMotherCanvas_Gen, *PdgIdCanvas2D_Gen;
    InitHistoBC(hMass_tripl_BC, hChi2Vertex, hMassvsChi2, hMass_quad_BC, hMass_quad_Zero_BC, hMass_di_Zero_BC, hMass_di_Zero2_BC, hPtRes_BC, hPtRes_BC_mu, hPtResBarrel_BC, hPtResBarrel_BC_mu, hPtResEndcap_BC, hPtResEndcap_BC_mu, hPdgId_Gen, hMotherPdgId_Gen, hPdgId2D_Gen);
    fout->cd();
    // Creation of histograms for variables AFTER cuts
    TDirectory *dirAfterCuts = fout->mkdir("AfterCuts");
    dirAfterCuts->cd();
    TH1I *hNtripl; TH1F *hChi2Track, *hmassQuad, *hmassQuad_Zero;
    TH1D *hMassTriRes, *hMassTriResBarrel, *hMassTriResEndcap, *hmassdi, *hPtRes_AC, *hPtRes_AC_mu[NMU_C], *hPtResBarrel_AC, *hPtResBarrel_AC_mu[NMU_C], *hPtResEndcap_AC, *hPtResEndcap_AC_mu[NMU_C], *hNMatchedStat, *hFlightDist, *hFlightDist_Signif, *hPtErrOverPt, *hPt_tripl_good, *hPt_tripl_fake, *hDeltaX, *hDeltaY, *hDeltaZ, *hDeltaX_fake, *hDeltaY_fake, *hDeltaZ_fake; TH2D *hFlightDistvsP;
    InitHistoAC(hNtripl, hChi2Track, hMassTriRes, hMassTriResBarrel, hMassTriResEndcap, hmassdi, hmassQuad, hmassQuad_Zero, hPtRes_AC, hPtRes_AC_mu, hPtResBarrel_AC, hPtResBarrel_AC_mu, hPtResEndcap_AC, hPtResEndcap_AC_mu, hNMatchedStat, hFlightDist, hFlightDist_Signif, hFlightDistvsP, hPtErrOverPt, hPt_tripl_good, hPt_tripl_fake, hDeltaX, hDeltaY, hDeltaZ, hDeltaX_fake, hDeltaY_fake, hDeltaZ_fake);
    fout->cd();
    // Creation of histo StepByStep
    TDirectory *dirStepByStep = fout->mkdir("StepByStep");
    dirStepByStep->cd();
    TDirectory *dirSingleMu = dirStepByStep->mkdir("SingleMu"); // Single mu variables histo
    dirSingleMu->cd();
    TH1D *hPt[NCUTS], *hPt_mu[NCUTS][NTOT], *hEta[NCUTS], *hEta_mu[NCUTS][NTOT], *hPhi[NCUTS], *hVx[NCUTS], *hVy[NCUTS], *hVz[NCUTS];
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
    TH1I *hCutEff = new TH1I("CutEff_Ntriplets", "CutEff_Ntriplets", NCUTS , 0.5, (NCUTS+0.5));
    TH1I *hCutEffEvt = new TH1I("CutEff_NEvents", "CutEff_NEvents", NCUTS , 0.5, (NCUTS+0.5));
    
    //Loop over the events
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        ntripl = 0; trInd = 0; int cutevt2[NCUTS] = {0};
        Long64_t ientry = fChain->LoadTree(jentry);
        fChain->GetEntry(ientry);
        //Loop over the TRIPLETS
        for (int j=0; j<TripletVtx2_Chi2->size(); j++){
            //Check on the number of tracks in the primary vertex
            if(RefittedPV2_NTracks->at(j) > 3){
                ntripltot++; Ncut = 0;
                // BEFORE cuts
                //Matching between index of the 2 mu (mu#_Ind) & that of 'MUONID' (mu#)
                //and that of the track of the triplet
                MatchIndex("ID", j, mu_Ind, mu);
                // Fill histograms
                FillHistoBC("MC", j, hMass_tripl_BC, hChi2Vertex, hMassvsChi2, hMass_quad_BC, hMass_quad_Zero_BC, hMass_di_Zero_BC, hMass_di_Zero2_BC, hPtRes_BC, hPtRes_BC_mu, hPtResBarrel_BC, hPtResBarrel_BC_mu, hPtResEndcap_BC, hPtResEndcap_BC_mu, IdsummaryDaughter_Gen, IdsummaryMother_Gen, Idsummary2D_Gen);
                FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                //CUT 1 : check that both mu are different from the track
                if(DuplicateFinder(Mu01_Eta->at(mu_Ind[0]), Mu02_Eta->at(mu_Ind[1]), Tr_Eta->at(mu_Ind[2]), Mu01_Phi->at(mu_Ind[0]), Mu02_Phi->at(mu_Ind[1]), Tr_Phi->at(mu_Ind[2]), Mu01_Pt->at(mu_Ind[0]), Mu02_Pt->at(mu_Ind[1]), Tr_Pt->at(mu_Ind[2])) == true){
                    Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                    FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                    //CUT 2 : check condition on Chi2 vertex ( 0 < Chi2 < 15)
                    if (TripletVtx2_Chi2->at(j) < TripletVtx_Chi2max && TripletVtx2_Chi2->at(j) > 0 ){
                        Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                        FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                        //CUT 3 : check condition 2 mu w/ opposite charge
                        double dimu1_2 = DimuonMass(MuonCharge->at(mu[0]), MuonCharge->at(mu[1]), Mu01_Pt->at(mu_Ind[0]), Mu02_Pt->at(mu_Ind[1]), Mu01_Eta->at(mu_Ind[0]), Mu02_Eta->at(mu_Ind[1]), Mu01_Phi->at(mu_Ind[0]), Mu02_Phi->at(mu_Ind[1]));
                        if (dimu1_2 != 0){
                            Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                            FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                            //CUT 4 : check condition on invariant dimuon mass (1-1.04) GeV
                            if (dimu1_2 >= 1 && dimu1_2 <= 1.04){
                                Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                //CUT 5 : check condition on IP of the track
                                if(Track_dz->at(mu[2]) < 20){
                                    Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                    FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                    //CUT 6 : check condition on IP of the track
                                    if (Track_dxy->at(mu[2]) < 0.3){
                                        Ncut++; cut[Ncut]++; cutevt2[Ncut]++;
                                        FillHistoStepByStep("MC", j, mu_Ind, mu, Ncut, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
                                        // CUT 7 : Null at the moment
                                        if(1 == 1){
                                            Ncut++; ntripl++; triplIndex[trInd] = j; trInd++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } // end loop on triplets
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
            Fill_DimuonMass(mu_Ind, mu, dimu);
            FillHistoStepByStep("MC", ind, mu_Ind, mu, NCUTS-1, hPt, hPt_mu, hEta, hEta_mu, hPhi, hVx, hVy, hVz, hPt_tripl, hEta_tripl, hPhi_tripl, hMass_tripl, IdsummaryDaughter, IdsummaryMother, Idsummary2D);
            // Resolution & final histograms
            MatchIndex("Gen", ind, mu_Ind, muGen);
            if(muGen[0] != -999 && muGen[1] != -999) {
                NgoodTripl++;
                FillHistoResoPt_AC(muGen, hPtRes_AC, hPtRes_AC_mu, hPtResBarrel_AC, hPtResBarrel_AC_mu, hPtResEndcap_AC, hPtResEndcap_AC_mu);
                StudyOnTriplet("good", ind, mu, hDeltaX, hDeltaY, hDeltaZ, hPt_tripl_good);
            }
            if(muGen[0] == -999 && muGen[1] == -999) {
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

// #########################################  END DsPhiPi ANALYSIS CUTFLOW
