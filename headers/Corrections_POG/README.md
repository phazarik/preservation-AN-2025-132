# POG corrections
Source: https://cms-analysis-corrections.docs.cern.ch/ 

## EGM
Correction JSONs are taken from the central repository.  We have used the following.

### Electron ID and Isolation scale factors
```
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2016preVFP-UL-NanoAODv9/2024-07-02/electron.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2016postVFP-UL-NanoAODv9/2024-07-02/electron.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2017-UL-NanoAODv9/2024-07-02/electron.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2018-UL-NanoAODv9/2024-07-02/electron.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22CDSep23-Summer22-NanoAODv12/2025-12-15/electron.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22EFGSep23-Summer22EE-NanoAODv12/2025-12-15/electron.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23CSep23-Summer23-NanoAODv12/2025-12-15/electron.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23DSep23-Summer23BPix-NanoAODv12/2025-12-15/electron.json.gz
```

### Electron scale and smearing scale factors
![Work in Progress](https://img.shields.io/badge/status-work_in_progress-yellow)

We are using the recommended ET-dependent corrections for Run 3. 
Source: [CMS-NOTE-2024-009](https://cds.cern.ch/record/2920027?ln=en) (Lepton energy scale and resolution corrections based on an analytical likelihood approach)
For Run 2, we are using the ones derived from NanoAODv9.
```
https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2016postVFP_UL/EGM_ScaleUnc.json.gz
https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2016preVFP_UL/EGM_ScaleUnc.json.gz
https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2017_UL/EGM_ScaleUnc.json.gz
https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2018_UL/EGM_ScaleUnc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22CDSep23-Summer22-NanoAODv12/latest/electronSS_EtDependent.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22EFGSep23-Summer22EE-NanoAODv12/latest/electronSS_EtDependent.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23CSep23-Summer23-NanoAODv12/latest/electronSS_EtDependent.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23DSep23-Summer23BPix-NanoAODv12/latest/electronSS_EtDependent.json.gz
```

## JME
```
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2016preVFP-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2016postVFP-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2017-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2018-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-22CDSep23-Summer22-NanoAODv12/2025-09-23/jet_jerc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-22EFGSep23-Summer22EE-NanoAODv12/2025-10-07/jet_jerc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-23CSep23-Summer23-NanoAODv12/2025-10-07/jet_jerc.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-23DSep23-Summer23BPix-NanoAODv12/2025-10-07/jet_jerc.json.gz
```
We have used the following keys to extract the scale factors.
  1. JEC keys
  - `Summer19UL16APV_V7_MC_Total_AK4PFchs` (v2)
  - `Summer19UL16_V7_MC_Total_AK4PFchs` (v2)
  - `Summer19UL17_V5_MC_Total_AK4PFchs` (v2)
  - `Summer19UL18_V5_MC_Total_AK4PFchs` (v2)
  - `Summer22_22Sep2023_V3_MC_Total_AK4PFPuppi` (v3)
  - `Summer22EE_22Sep2023_V3_MC_Total_AK4PFPuppi` (v3)
  - `Summer23Prompt23_V2_MC_Total_AK4PFPuppi` (v2)
  - `Summer23BPixPrompt23_V3_MC_Total_AK4PFPuppi` (v2)

  2. JER keys
  - `Summer20UL16APV_JRV3_MC_ScaleFactor_AK4PFchs` (v2)
  - `Summer20UL16_JRV3_MC_ScaleFactor_AK4PFchs` (v2)
  - `Summer19UL17_JRV2_MC_ScaleFactor_AK4PFchs` (v2)
  - `Summer19UL18_JRV2_MC_ScaleFactor_AK4PFchs` (v2)
  - `Summer22_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi` (v3)
  - `Summer22EE_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi` (v3)
  - `Summer23Prompt23_RunCv1234_JRV1_MC_ScaleFactor_AK4PFPuppi` (v2)
  - `Summer23BPixPrompt23_RunD_JRV1_MC_ScaleFactor_AK4PFPuppi` (v2)

  3. pT-resolution keys:
  - `Summer20UL16APV_JRV3_MC_PtResolution_AK4PFchs` (v2)
  - `Summer20UL16_JRV3_MC_PtResolution_AK4PFchs` (v2)
  - `Summer19UL17_JRV2_MC_PtResolution_AK4PFchs` (v2)
  - `Summer19UL18_JRV2_MC_PtResolution_AK4PFchs` (v2)
  - `Summer22_22Sep2023_JRV1_MC_PtResolution_AK4PFPuppi` (v3)
  - `Summer22EE_22Sep2023_JRV1_MC_PtResolution_AK4PFPuppi` (v3)
  - `Summer23Prompt23_RunCv1234_JRV1_MC_PtResolution_AK4PFPuppi` (v2)
  - `Summer23BPixPrompt23_RunD_JRV1_MC_PtResolution_AK4PFPuppi` (v2)

## LUM
```
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2016preVFP-UL-NanoAODv9/2021-09-10/puWeights.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2016postVFP-UL-NanoAODv9/2021-09-10/puWeights.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2017-UL-NanoAODv9/2021-09-10/puWeights.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2018-UL-NanoAODv9/2021-09-10/puWeights.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-22CDSep23-Summer22-NanoAODv12/2024-01-31/puWeights.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-22EFGSep23-Summer22EE-NanoAODv12/2024-01-31/puWeights.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-23CSep23-Summer23-NanoAODv12/2024-01-31/puWeights.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-23DSep23-Summer23BPix-NanoAODv12/2024-01-31/puWeights.json.gz
```
Keys used in different runs are listed below.
 - 2016: `Collisions16_UltraLegacy_goldenJSON` (v0)
 - 2017: `Collisions17_UltraLegacy_goldenJSON` (v0)
 - 2018: `Collisions18_UltraLegacy_goldenJSON` (v0)
 - 2022-preEE:    `Collisions2022_355100_357900_eraBCD_GoldenJson` (v0)
 - 2022-postEE:   `Collisions2022_359022_362760_eraEFG_GoldenJson` (v0)
 - 2023-preBPix:  `Collisions2023_366403_369802_eraBC_GoldenJson` (v0)
 - 2023-postBPix: `Collisions2023_369803_370790_eraD_GoldenJson` (v0)

## MUO
```
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2016preVFP-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2016postVFP-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2017-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2018-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-22CDSep23-Summer22-NanoAODv12/2025-08-14/muon_Z.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-22EFGSep23-Summer22EE-NanoAODv12/2025-08-14/muon_Z.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-23CSep23-Summer23-NanoAODv12/2025-08-14/muon_Z.json.gz
/cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-23DSep23-Summer23BPix-NanoAODv12/2025-08-14/muon_Z.json.gz
```
Keys used for extracting reco, Iso and Iso scale factors are listed below. HLT scale factors are not used from here.
 - **Run2:** `NUM_TrackerMuons_DEN_genTracks` (reco), `NUM_MediumID_DEN_genTracks` (ID), `NUM_TightRelIso_DEN_MediumID` (Iso)
 - **Run3:** `None` (reco), `NUM_MediumID_DEN_TrackerMuons` (ID), `NUM_TightPFIso_DEN_MediumID` (Iso)
  