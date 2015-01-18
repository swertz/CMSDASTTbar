#include <CSVReweighter.hpp>

#include <TFile.h>

#include <cstdlib>
#include <string>
#include <stdexcept>
#include <sstream>
/**/#include <iostream>


using namespace std;


// Static data members
unsigned const CSVReweighter::nPtBinsHF;
unsigned const CSVReweighter::nPtBinsLF;
unsigned const CSVReweighter::nEtaBinsLF;


CSVReweighter::CSVReweighter()
{
    // Find the installation path of the package
    char const *installPathPointer = getenv("CMS_DAS_TTbar_INSTALL");
    
    if (not installPathPointer)
        throw runtime_error("Environmental variable \"CMS_DAS_TTbar_INSTALL\" is not defined.");
    
    string installPath(installPathPointer);
    
    if (installPath.length() > 0 and installPath[installPath.length() - 1] != '/')
        installPath += '/';
    
    string const dataPath(installPath + "Reader/data/");
    
    
    // Open data files that contain histograms for CSV reweighting
    unique_ptr<TFile> dataFileHF(TFile::Open((dataPath + "csv_rwt_hf.root").c_str()));
    unique_ptr<TFile> dataFileLF(TFile::Open((dataPath + "csv_rwt_lf.root").c_str()));
    
    if (not dataFileHF or dataFileHF->IsZombie())
        throw runtime_error(string("Data file \"") + dataPath +
         "csv_rwt_hf.root\" does not exist or is corrupted.");
    
    if (not dataFileLF or dataFileLF->IsZombie())
        throw runtime_error(string("Data file \"") + dataPath +
         "csv_rwt_lf.root\" does not exist or is corrupted.");
    
    
    // Read histograms for heavy flavours
    for (unsigned iPt = 0; iPt < nPtBinsHF; ++iPt)
    {
        ostringstream nameFragment;
        nameFragment << "csv_ratio_Pt" << iPt << "_Eta0_final";
        
        weightsBottom[Systematics::Nominal][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get(nameFragment.str().c_str())));
        weightsBottom[Systematics::JECUp][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_JESUp").c_str())));
        weightsBottom[Systematics::JECDown][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_JESDown").c_str())));
        weightsBottom[Systematics::PurityHFUp][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_LFUp").c_str())));
        weightsBottom[Systematics::PurityHFDown][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_LFDown").c_str())));
        weightsBottom[Systematics::StatHF1Up][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_Stats1Up").c_str())));
        weightsBottom[Systematics::StatHF1Down][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_Stats1Down").c_str())));
        weightsBottom[Systematics::StatHF2Up][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_Stats2Up").c_str())));
        weightsBottom[Systematics::StatHF2Down][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((nameFragment.str() + "_Stats2Down").c_str())));
        
        weightsCharm[Systematics::Nominal][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((string("c_") + nameFragment.str()).c_str())));
        weightsCharm[Systematics::CharmUnc1Up][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((string("c_") + nameFragment.str() + "_cErr1Up").c_str())));
        weightsCharm[Systematics::CharmUnc1Down][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((string("c_") + nameFragment.str() + "_cErr1Down").c_str())));
        weightsCharm[Systematics::CharmUnc2Up][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((string("c_") + nameFragment.str() + "_cErr2Up").c_str())));
        weightsCharm[Systematics::CharmUnc2Down][iPt].reset(reinterpret_cast<TH1D *>(
         dataFileHF->Get((string("c_") + nameFragment.str() + "_cErr2Down").c_str())));
    }
    
    
    // Make sure all histograms for heavy flavours have been read
    for (auto const &wp: weightsBottom)
        for (unsigned iPt = 0; iPt < nPtBinsHF; ++iPt)
            if (not wp.second[iPt])
            {
                ostringstream ost;
                ost << "Cannot find histogram for b quarks, systematical variation #" <<
                 unsigned(wp.first) << ", pt bin #" << iPt << ".";
                 
                throw runtime_error(ost.str());
            }
    
    for (auto const &wp: weightsCharm)
        for (unsigned iPt = 0; iPt < nPtBinsHF; ++iPt)
            if (not wp.second[iPt])
            //^ There is no statistical uncertainty for charm weights
            {
                ostringstream ost;
                ost << "Cannot find histogram for c quarks, systematical variation #" <<
                 unsigned(wp.first) << ", pt bin #" << iPt << ".";
                 
                throw runtime_error(ost.str());
            }
    
    
    // Disentangle the histograms for heavy flavours from their parent file, so that the histograms
    //will not be deleted when the file is closed
    for (auto const &wp: weightsBottom)
        for (unsigned iPt = 0; iPt < nPtBinsHF; ++iPt)
            wp.second[iPt]->SetDirectory(nullptr);
    
    for (auto const &wp: weightsCharm)
        for (unsigned iPt = 0; iPt < nPtBinsHF; ++iPt)
                wp.second[iPt]->SetDirectory(nullptr);
    
    
    // Read histograms for light flavours
    for (unsigned iPt = 0; iPt < nPtBinsLF; ++iPt)
        for (unsigned iEta = 0; iEta < nEtaBinsLF; ++iEta)
        {
            ostringstream nameFragment;
            nameFragment << "csv_ratio_Pt" << iPt << "_Eta" << iEta << "_final";
            
            weightsLight[Systematics::Nominal][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get(nameFragment.str().c_str())));
            weightsLight[Systematics::JECUp][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_JESUp").c_str())));
            weightsLight[Systematics::JECDown][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_JESDown").c_str())));
            weightsLight[Systematics::PurityLFUp][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_HFUp").c_str())));
            weightsLight[Systematics::PurityLFDown][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_HFDown").c_str())));
            weightsLight[Systematics::StatLF1Up][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_Stats1Up").c_str())));
            weightsLight[Systematics::StatLF1Down][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_Stats1Down").c_str())));
            weightsLight[Systematics::StatLF2Up][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_Stats2Up").c_str())));
            weightsLight[Systematics::StatLF2Down][iPt][iEta].reset(reinterpret_cast<TH1D *>(
             dataFileLF->Get((nameFragment.str() + "_Stats2Down").c_str())));
        }
    
    
    // Make sure all histograms for light flavours have been read
    for (auto const &wp: weightsLight)
        for (unsigned iPt = 0; iPt < nPtBinsLF; ++iPt)
            for (unsigned iEta = 0; iEta < nEtaBinsLF; ++iEta)
            {
                if (not wp.second[iPt][iEta])
                {
                    ostringstream ost;
                    ost << "Cannot find histogram for light-flavour quarks, systematical " <<
                     "variation #" << unsigned(wp.first) << ", pt bin #" << iPt << ".";
                     
                    throw runtime_error(ost.str());
                }
            }
    
    
    // Disentangle the histograms for light flavours from their parent file
    for (auto const &wp: weightsLight)
        for (unsigned iPt = 0; iPt < nPtBinsLF; ++iPt)
            for (unsigned iEta = 0; iEta < nEtaBinsLF; ++iEta)
                wp.second[iPt][iEta]->SetDirectory(nullptr);
}


double CSVReweighter::CalculateJetWeight(Jet const &jet,
 Systematics syst /*= Systematics::Nominal*/) const
{
    // Find pt and eta bins into which the given jet falls
    int iPt = -1, iEta = -1;
    double const pt = jet.Pt();
    double const absEta = fabs(jet.Eta());
    
    for (double const &cut: {20., 30., 40., 60., 100., 160.})
    {
        if (pt < cut)
            break;
        
        ++iPt;
    }
    
    for (double const &cut: {0., 0.8, 1.6, 2.4})
    {
        if (absEta < cut)
            break;
        
        ++iEta;
    }
    
    
    // If the jet is out of the supported range, return a unit weight
    if (iPt < 0 or iPt >= int(nPtBinsHF) or iEta < 0 or iEta >= int(nEtaBinsLF))
        return 1.;
    
    
    // Calculate the per-jet weight depending on the jet flavour
    double const csv = jet.BTag();
    
    switch (abs(jet.Flavour()))
    {
        case 5:  // b-quark jets
        {
            // Check for mismatched systematical variation
            if (weightsBottom.find(syst) == weightsBottom.end())
                syst = Systematics::Nominal;
            
            int const bin = (csv >= 0.) ? weightsBottom[syst][iPt]->FindFixBin(csv) : 1;
            return weightsBottom[syst][iPt]->GetBinContent(bin);
            
            break;
        }
        
        case 4:  // c-quark jets
        {
            // Check for mismatched systematical variation
            if (weightsCharm.find(syst) == weightsCharm.end())
                syst = Systematics::Nominal;
            
            int const bin = (csv >= 0.) ? weightsCharm[syst][iPt]->FindFixBin(csv) : 1;
            return weightsCharm[syst][iPt]->GetBinContent(bin);
            
            break;
        }
        
        default:  // light-flavour jets
        {
            // The pt range for light-flavour is different. Clip the pt accordingly
            if (iPt > 3)
                iPt = 3;
            
            // Check for mismatched systematical variation
            if (weightsLight.find(syst) == weightsLight.end())
                syst = Systematics::Nominal;
            
            int const bin = (csv >= 0.) ? weightsLight[syst][iPt][iEta]->FindFixBin(csv) : 1;
            return weightsLight[syst][iPt][iEta]->GetBinContent(bin);
            
            break;
        }
    }
}
