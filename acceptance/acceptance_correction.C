//Loop over all histograms;
//for each hist,
//plot recon vs thrown,
//acceptance, and acceptance corrected data
// all in one page

//Author: Hao Li
//Email: hl2@andrew.cmu.edu 
//Date: 6-5-2017	




#include "TCanvas.h"
#include "TString.h"
#include <iostream>
#include "TFile.h"
#include "TH1I.h"
#include "TH1D.h"
#include "TH2I.h"
#include "TH2D.h"
#include "TKey.h"
#include "TMath.h"


using namespace std;

void print_histo(TFile * f1, TFile * f2, TFile *f3, TString path, TString newTreePath)
{
	gStyle->SetOptTitle(1);
  	gStyle->SetOptStat("nem");   //show "name" and "entries" only
  	gStyle->SetTitleFontSize(.04);
 	gStyle->SetLabelSize(.04, "XY");

	TFile * h1 = f1;
	TFile * h2 = f2;
	TFile * h3 = f3;
	//Define a canvas
	TCanvas* canvas = new TCanvas("canvas");
	TString pdfPageOne = path;
	TRegexp repdf1("pdf");
	pdfPageOne(repdf1) = "pdf(";
	canvas->Print(pdfPageOne,"Title: Cover");

	//create an iterator
	TIter nextkey( h1->GetListOfKeys() );
	TKey *key;
	int histocounter = 0;
	double fscale;
	//Loop over the keys in the file
	while ( (key = (TKey*)nextkey()))
	{
		// Define the Canvas
	    TCanvas *c = new TCanvas("c", "canvas", 1200, 800);
	    //c->SetBottomMargin(0.15);
	    
	
		//std::cout <<"KEY: "<<key->GetClassName()<<"\t"<<key->GetName()<<";"<<key->GetCycle()<<"\t"<<key->GetTitle()<<std::endl;
		//Read object from first source file
		TObject *obj = key->ReadObj();
		
		if ( strcmp(key->GetClassName(), "TH2I") == 0 ) continue;
		if ( strcmp(key->GetClassName(), "TH2D") == 0 ) continue;
		//Check that the object the key points to is a histogram.
		if( obj->IsA()->InheritsFrom("TH1"))
		{
			//count the number of histograms printed
			histocounter++;
			cout<<histocounter<<endl;

			//cast string for title in pdf's content table. 
			  //NOTE: the title string that contains "vertex" should be avoided.
			TString title = Form("Title: %s",key->GetName());
			TString keyname = Form("%s",key->GetName());
			if(title.Contains("tex"))
				{
					//use regexp to search and replace the string "tex" with "tx"
					TRegexp re("tex");
					title(re) = "tx";
				}
			//if(title.Contains("BeamEnergy")) continue;
			//std::cout<< title <<std::endl;


			//Cast the TObject pointer to a histogram one. Different classes of histos should be treated accordingly, or information will be lost.
			TH1 *histo;
			TH1 *histo_mc;
			TH1 *histo_data;

			if ( strcmp(key->GetClassName(), "TH1I") == 0 ) 
				{
					histo = (TH1I*)(obj);
					histo_mc = (TH1I*) h2->Get(key->GetName());
					histo_data = (TH1I*) h3->Get(key->GetName());
				}
			if ( strcmp(key->GetClassName(), "TH1D") == 0 ) 
				{
					histo = (TH1D*)(obj);
					histo_mc = (TH1D*) h2->Get(key->GetName());
					histo_data = (TH1D*) h3->Get(key->GetName());
				}

			if(histo_mc == NULL) continue;
			histo->GetXaxis()->SetTitleSize(0.04);

					
			//fscale = (histo_mc->Integral())/(histo->Integral());
			//histo -> Scale(fscale);
			histo_mc -> SetLineColor(kRed);

			if(histo->GetMaximum() > histo_mc->GetMaximum())
			{
				histo->GetYaxis()->SetRangeUser(0, 1.1*(histo->GetMaximum()));
			}
			else
			{
				histo->GetYaxis()->SetRangeUser(0, 1.1*(histo_mc->GetMaximum()));
			}

			// Upper plot will be in pad1
		    TPad *pad1 = new TPad("pad1", "pad1", 0.05, 0.45, 0.5, 1.0);
		    pad1->SetBottomMargin(0); // Upper and lower plot are joined
		    pad1->SetGridx();         // Vertical grid
		    pad1->Draw();             // Draw the upper pad: pad1
		    pad1->cd();               // pad1 becomes the current pad

		    histo->SetLineStyle(7);
		    histo_mc->SetLineStyle(7);

			histo->Draw("HIST");
			histo_mc->Draw("sameS HIST");
			

			c->Update();
			TPaveStats *ps1;
			
			ps1 = (TPaveStats*)histo_mc->FindObject("stats");
				

			if(ps1 != NULL)
			{
				ps1->SetName("mystats");
					ps1->SetX1NDC(0.58); ps1->SetX2NDC(0.78);
					ps1->SetY1NDC(0.815); ps1->SetY2NDC(0.935);
			}

			TLegend * leg1 = new TLegend(0.7,0.68,0.95,0.8);
    		leg1->AddEntry(histo, "thrown MC");
    		leg1->AddEntry(histo_mc, "reconstructed MC");
    		leg1->Draw();
  					
  				
			// lower plot will be in pad
			   c->cd();          // Go back to the main canvas before defining pad2
			   TPad *pad2 = new TPad("pad2", "pad2", 0.05, 0.05, 0.5, 0.45);
			   pad2->SetTopMargin(0);
			   //pad2->SetBottomMargin(0.1);
			   pad2->SetGridx(); // vertical grid
			   pad2->SetGridy(); // horizontal grid
			   pad2->Draw();
			   pad2->cd();       // pad2 becomes the current pad
				


			
			   //use brutal force to fill the ratio histogram bin by bin
			   int Nbin = histo->GetNbinsX();
			   double hmax = histo->GetXaxis()->GetBinUpEdge(Nbin);
			   double hmin = histo->GetXaxis()->GetBinLowEdge(1);
			   //cout<<Nbin<<"  "<<hmin<<"  "<<hmax<<endl;
			   // Define the ratio plot
			   //TH1F *h3 = (TH1F*)histo->Clone("h3");
			   TH1D *h3 = new TH1D("h3","ratio;reconstructed MC/generator thrown MC", Nbin, hmin, hmax);
			   TH1D *h4 = new TH1D("h4","acceptance corrected data", Nbin, hmin, hmax);
			   for(int index=1; index<=Nbin; index++)
				   {
				   		double ratio, binEr;
				   		double binContent_data, binContent_dataCorrected, binEr_data, binEr_dataCorrected;
				   		if(histo->GetBinContent(index) == 0) 
					   		{
					   			ratio = 0.0;
					   			binEr = 0.0;
					   			binContent_dataCorrected = 0.0;
					   			binEr_dataCorrected =0.0;
					   		}
				   		else
				   			{
				   				ratio = 1.0*(histo_mc->GetBinContent(index))/(histo->GetBinContent(index));
				   				
				   		 		binEr = histo_mc->GetBinError(index)/(histo->GetBinContent(index));
				   		 		double epsilon = binEr/ratio;
				   		 		if(ratio < 0.01 || epsilon > 0.2) 
				   		 			{
				   		 				ratio=0.0;
				   		 				binEr=0.0;
				   		 			}
				   		 		

				   		 		binContent_data = 1.0 * (histo_data->GetBinContent(index));
				   		 		binEr_data = histo_data->GetBinError(index);

				   		 		if(ratio != 0 && binContent_data != 0)
				   		 			{
				   		 				binContent_dataCorrected = binContent_data/ratio;
					   					binEr_dataCorrected = binContent_dataCorrected * TMath::Sqrt(TMath::Power((binEr_data/binContent_data),2.0) + TMath::Power((binEr/ratio),2.0));
				   		 			}
			   		 			else
			   		 				{
			   		 					if(ratio == 0 || binContent_data==0) 
			   		 						{
			   		 							binContent_dataCorrected = 0.0;
			   		 							binEr_dataCorrected = 0.0;
			   		 						}
				   						 

			   		 				}
			   		 				
				   		 	}
				   		h3->SetBinContent(index, ratio);
				   		h3->SetBinError(index, binEr);

				   		h4->SetBinContent(index, binContent_dataCorrected);
				   		h4->SetBinError(index, binEr_dataCorrected);

				   		//if(index == 20) cout<< key->GetName() <<index <<"    "<<ratio<<"   "<<binEr<<"   "<<h3->GetBinContent(index)<<endl;
				   }

			   h3->UseCurrentStyle();
			   h3->SetLineColor(kRed);
			   h3->SetStats(0);      // No statistics on lower plot
			   h3->GetYaxis()->SetRangeUser(-0.01, 1.0 );
			   h3->SetMarkerStyle(1);
			   h3->SetMarkerSize(2);
			   h3->SetMarkerColor(kRed);
			   h3->Draw("Ep");       // Draw the ratio plot
			   //cout<<h3->GetBinContent(20)<<endl;
				   
			   // Ratio plot (h3) settings
   			   h3->SetTitle(""); // Remove the ratio title




   			   // lower plot will be in pad
			   c->cd();          // Go back to the main canvas before defining pad2
			   TPad *pad3 = new TPad("pad3", "pad3", 0.55, 0.63, 1.0, 1.0);
			   //pad3->SetTopMargin(0);
			   //pad2->SetBottomMargin(0.1);
			   pad3->SetGridx(); // vertical grid
			   //pad3->SetGridy(); // horizontal grid
			   pad3->Draw();
			   pad3->cd();       // pad2 becomes the current pad

			   histo_data->GetYaxis()->SetRangeUser(0, 1.1*(h4->GetMaximum()));
			   histo_data->SetLineColor(kRed);
			   h4->SetLineColor(kBlue);
			   h4->SetStats(0);      // No statistics on lower plot
			   

			   histo_data->Draw("HIST");
			   h4->Draw("sameS HIST");

			   TLegend * leg2 = new TLegend(0.7,0.68,0.95,0.8);
    			leg2->AddEntry(histo_data, "data");
    			leg2->AddEntry(h4, "corrected data");
    			leg2->Draw();


			    // lower plot will be in pad
			   c->cd();          // Go back to the main canvas before defining pad2
			   TPad *pad4 = new TPad("pad4", "pad4", 0.55, 0.2, 1.0, 0.6);
			   //pad4->SetTopMargin(0);
			   pad4->SetBottomMargin(0);
			   pad4->SetGridx(); // vertical grid
			   //pad4->SetGridy(); // horizontal grid
			   pad4->Draw();
			   pad4->cd();       // pad2 becomes the current pad


			   double rescale;
			   TH1F *histo_CorrectedData = (TH1F*)h4->Clone("histo_CorrectedData");
			   TH1F *histo_thrown = (TH1F*)histo->Clone("histo_thrown");
			   rescale = (histo_CorrectedData->Integral())/(histo_thrown->Integral());
			   histo_thrown -> Scale(rescale);

			   if(histo_CorrectedData->GetMaximum() > histo_thrown->GetMaximum())
					{
						histo_CorrectedData->GetYaxis()->SetRangeUser(0, 1.1*(histo_CorrectedData->GetMaximum()));
					}
					else
					{
						histo_CorrectedData->GetYaxis()->SetRangeUser(0, 1.1*(histo_thrown->GetMaximum()));
					}

			   histo_CorrectedData->UseCurrentStyle();
			   histo_CorrectedData->SetLineColor(kRed);
			   histo_CorrectedData->SetMarkerColor(kRed);
			   histo_thrown->SetLineColor(kBlack);
			   histo_thrown->SetLineStyle(7);
			   //histo_CorrectedData->SetStats(0);      // No statistics on lower plot

			   histo_CorrectedData->Draw("EP");
			   histo_thrown->Draw("sameS EP");

			   TLegend * leg3 = new TLegend(0.7,0.68,0.95,0.8);
				
    			leg3->AddEntry(histo_CorrectedData, "corrected data");
    			leg3->AddEntry(histo_thrown, "thrown MC");
    			leg3->Draw();

			   
			   // lower plot will be in pad
			   c->cd();          // Go back to the main canvas before defining pad2
			   TPad *pad5 = new TPad("pad5", "pad5", 0.55, 0.0, 1.0, 0.2);
			   pad5->SetTopMargin(0);
			   pad5->SetBottomMargin(0.2);
			   pad5->SetGridx(); // vertical grid
			   pad5->SetGridy(); // horizontal grid
			   pad5->Draw();
			   pad5->cd();       // pad2 becomes the current pad

			   TH1F *h6 = (TH1F*)histo_CorrectedData->Clone("h6");
			   h6->Sumw2();
			   h6->Divide(histo_thrown);
			   h6->GetYaxis()->SetRangeUser(0.0, 2.0 );

			   h6->SetTitle("");
			   h6->SetStats(0); 
			   h6->Draw("EP");






			
			   c->Print(path, title);

			   //save the current directory
				//TDirectory *save_dir = gDirectory;
				
			   //save the corrected data to new tree file
			   TFile * newTree = new TFile(newTreePath,"UPDATE");
			   histo_CorrectedData->Write("");

			   //go back to wherever directory it is
			   //save_dir->cd();

   			   delete h3;
   			   delete h4;
   			   delete newTree;
   			   delete histo_CorrectedData;
   			   delete histo_thrown;
			   //delete pad1, pad2;
		



		}
		//for convenience of monitoring plotting activities in the terminal window
		//std::cout<<"----------------------------------"<<std::endl;


		
		c->Clear();
		delete c;
		//delete pad1;
		//delete pad2;
		
		
	}//end of the while loop

	//Close the .pdf file and delete the canvas
	canvas->Clear();

	

	TString pdfPageEnd = path;
	TRegexp repdf2("pdf");
	pdfPageEnd(repdf2) = "pdf)";
	canvas->Print(pdfPageEnd,"Title: The End");
	delete canvas;
	



}//end of the function





void recur_copy(TDirectory* source, TDirectory* target)
{

	TDirectory *new_dir = target;
	new_dir->pwd();
    source->cd();
	//create an iterator
	TIter nextkey( source->GetListOfKeys() );
	TKey *key;

	//Loop over the keys in the file
	while ( (key = (TKey*)nextkey()))
	{
		//std::cout <<"KEY: "<<key->GetClassName()<<"\t"<<key->GetName()<<";"<<key->GetCycle()<<"\t"<<key->GetTitle()<<std::endl;
		//Read object from first source file
		TObject *obj = key->ReadObj();
		TString obj_info = Form("TObject (%s): %.5s created.  ",key->GetClassName(),key->GetName());
		//std::cout << obj_info <<std::endl;
		
		//if the object os still a TDirectory, then call the function recursively.
		if ( strcmp(key->GetClassName(), "TDirectoryFile") == 0 )
		{
			recur_copy((TDirectory*)(obj), new_dir);
		}

		//Check that the object the key points to is a histogram.
		if ( obj->IsA()->InheritsFrom("TH1") )
		{
			

			//Cast the TObject pointer to a histogram one.
			TH1 *histo;
			if ( strcmp(key->GetClassName(), "TH1I") == 0 ) {histo = (TH1I*)(obj);}
			if ( strcmp(key->GetClassName(), "TH1D") == 0 ) {histo = (TH1D*)(obj);}
			if ( strcmp(key->GetClassName(), "TH2I") == 0 ) {histo = (TH2I*)(obj);}
			if ( strcmp(key->GetClassName(), "TH2D") == 0 ) {histo = (TH2D*)(obj);}

			TDirectory * histo_MotherDir = key->GetMotherDir();
			TString MotherDirID = histo_MotherDir->GetName();

			//to distinguish default histograms
			if(MotherDirID.Remove(0,MotherDirID.Length()-5) != ".root" )
				{
				TDirectory * histo_GrandMotherDir = histo_MotherDir->GetMotherDir();
				TString GrandMotherDirID = histo_GrandMotherDir->GetName();

				TString stepNumA, stepNumB;
				stepNumA = GrandMotherDirID[4];
				stepNumB = Form("0");
				
				TString histo_name = Form("%c_%.6s_%s",GrandMotherDirID[4], histo_MotherDir->GetName(), key->GetName() );
				if(stepNumA == stepNumB)
				{
					TDirectory * histo_GrandGrandMotherDir = histo_GrandMotherDir->GetMotherDir();
					TString GrandGrandMotherDirID = histo_GrandGrandMotherDir->GetName();
					if(GrandGrandMotherDirID.Contains("Hist_ParticleComboKinematics")) GrandGrandMotherDirID.Remove(0,29);
					if(GrandGrandMotherDirID.Contains("Hist_ParticleID")) GrandGrandMotherDirID.Remove(0,16);
					histo_name.Prepend("_Step");
					histo_name.Prepend(GrandGrandMotherDirID);

				}
				//std::cout<< histo_name <<std::endl;
				histo->SetName(histo_name);
				}
			//save the current directory
			TDirectory *save_dir = gDirectory;
			//copy the histogram over to the new file
			new_dir->cd();
			histo->Write();
			save_dir->cd();
		}
		//For convenience of monitoring
		//std::cout<<"----------------------------------"<<std::endl;
		
	}



}

void copy_histo(const char *filename)
{
	//copy all histograms into current directory
	gDirectory->pwd();
	TDirectory *target = gDirectory;

	//open the .root file
	TFile * f1 = new TFile(filename);
	TDirectory *old_dir = gDirectory;
	//check
	if (!f1 || f1->IsZombie()) 
	{
      printf("Cannot copy file: %s\n",filename);
      target->cd();
      return;
   	}
   	target->cd();
   	recur_copy(old_dir, target);
   	target->cd();
}
 

void acceptance_correction(TString gen_path  = "/home/haoli/thrown_Submit/hist_ppbar3_mixedModel_12_11.root",
					 TString mc_path         = "/home/haoli/phaseSpaceMC/hist_ppbar3_recon_mixedModel_12_11.root",
					 TString data_path       = "/home/haoli/cluster/hist_totalSP17_12_11_papp.acc.root",
	                 TString plotpath        = "/home/haoli/analysis/gluex_root_analysis/plots/acceptance_correction_1_12_2018.pdf",
	                 TString newTreePath     = "/home/haoli/analysis/gluex_root_analysis/acceptanceCorrectionProduct/hist_totalSP17AcceptanceCorrected_1_12_2018_papp.root")
//void plot_histfile(TString path = "/home/haoli/cluster/hist_total_9_4_2016.root ",
	           //TString plotpath = "/home/haoli/analysis/gluex_root_analysis/plots/hist_total_e5_9_4_Run2016SP.pdf")
{
	//creat a new tree to save the acceptance corrected data
	TFile* newTree = new TFile(newTreePath,"RECREATE");
	//create new .root file for copying histograms (if there is a similar file, it will be overwritten)
	TFile* h1 = new TFile("/home/haoli/analysis/gluex_root_analysis/plots/hist_mcGen_temp_plots.root","RECREATE");
	gDirectory->pwd();
	
	std::cout<<"---------------------------------------------------------------------------------------"<<std::endl;

	//call the copy_histo() function recursively (i.e. loop over every histogram in all the subdirectories of the .root file) to copy every histogram in to a new .root file
	copy_histo(gen_path);


	TFile* h2 = new TFile("/home/haoli/analysis/gluex_root_analysis/plots/hist_mcRecon_RunSP2017_temp_plots.root","RECREATE");
	gDirectory->pwd();
	std::cout<<"---------------------------------------------------------------------------------------"<<std::endl;

	copy_histo(mc_path);

	TFile* h3 = new TFile("/home/haoli/analysis/gluex_root_analysis/plots/hist_data_temp_plots.root","RECREATE");
	gDirectory->pwd();
	
	std::cout<<"---------------------------------------------------------------------------------------"<<std::endl;

	//call the copy_histo() function recursively (i.e. loop over every histogram in all the subdirectories of the .root file) to copy every histogram in to a new .root file
	copy_histo(data_path);

	//avoid crash
	gROOT->Reset();

	//loop over the new .root file to plot everything in a .pdf file
	print_histo(h1, h2, h3, plotpath, newTreePath);
	


}


















