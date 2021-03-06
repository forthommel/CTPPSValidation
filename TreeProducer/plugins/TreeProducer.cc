// -*- C++ -*-
//
// Package:    CTPPSAnalysis/TreeProducer
// Class:      TreeProducer
// 
/**\class TreeProducer TreeProducer.cc CTPPSAnalysis/TreeProducer/plugins/TreeProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Laurent Forthomme
//         Created:  Mon, 24 Oct 2016 13:04:07 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

// strips
#include "DataFormats/CTPPSReco/interface/TotemRPLocalTrack.h"

// diamonds
#include "DataFormats/CTPPSDigi/interface/CTPPSDiamondDigi.h"
#include "DataFormats/CTPPSReco/interface/CTPPSDiamondRecHit.h"
#include "DataFormats/CTPPSDetId/interface/CTPPSDiamondDetId.h"

#include "TTree.h"
#include "TH2.h"

class TreeProducer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
  public:
    explicit TreeProducer( const edm::ParameterSet& );
    ~TreeProducer();

    static void fillDescriptions( edm::ConfigurationDescriptions& descriptions );

  private:
    virtual void beginJob() override;
    virtual void analyze( const edm::Event&, const edm::EventSetup& ) override;
    virtual void endJob() override;

    void clearTree();
    void extrapolateToDiamonds( const TotemRPLocalTrack&, float*, float* ) const;

    edm::EDGetTokenT< edm::DetSetVector<TotemRPLocalTrack> > stripsLocalTrackToken_;
    edm::EDGetTokenT< edm::DetSetVector<CTPPSDiamondDigi> > diamondDigiToken_;
    edm::EDGetTokenT< edm::DetSetVector<CTPPSDiamondRecHit> > diamondRecHitToken_;

    TTree* tree_;
    // strips leaves
    std::vector<unsigned short> strips_tracks_detid_;
    std::vector<float> strips_tracks_x0_, strips_tracks_y0_, strips_tracks_z0_;
    std::vector<float> strips_tracks_sigx0_, strips_tracks_sigy0_;
    std::vector<float> strips_tracks_xdiam_, strips_tracks_ydiam_;
    std::vector<float> strips_tracks_tx_, strips_tracks_ty_;
    std::vector<float> strips_tracks_sigtx_, strips_tracks_sigty_;
    // diamonds leaves
    std::vector<unsigned short> diam_digis_detid_, diam_digis_arm_, diam_digis_plane_, diam_digis_channelid_;
    std::vector<unsigned short> diam_digis_multiplehits_;
    std::vector<unsigned int> diam_digis_leadedge_, diam_digis_trailedge_, diam_digis_thresvolt_;

    std::vector<unsigned short> diam_rh_side_, diam_rh_plane_;
    std::vector<float> diam_rh_x_, diam_rh_y_;
    std::vector<float> diam_rh_x_size_, diam_rh_y_size_;

    TH2D* hitmap_trks_diam_plus_, *hitmap_trks_diam_minus_;
    TH2D* hitmap_trks_diam_plus_matchst0_, *hitmap_trks_diam_minus_matchst0_;
};

TreeProducer::TreeProducer( const edm::ParameterSet& iConfig ) :
  stripsLocalTrackToken_( consumes< edm::DetSetVector<TotemRPLocalTrack> > ( iConfig.getParameter<edm::InputTag>( "stripsLocalTrackLabel" ) ) ),
  diamondDigiToken_     ( consumes< edm::DetSetVector<CTPPSDiamondDigi> >  ( iConfig.getParameter<edm::InputTag>( "diamondDigiLabel" ) ) ),
  diamondRecHitToken_   ( consumes< edm::DetSetVector<CTPPSDiamondRecHit> >( iConfig.getParameter<edm::InputTag>( "diamondRecHitLabel" ) ) )
{
  usesResource( "TFileService" );
  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>( "tree", "" );
  hitmap_trks_diam_plus_ = fs->make<TH2D>( "hitmap_trks_diam_plus", "Proton hits at first diamond plane (z > 0, extrapolated from strips tracks);x (m);y (m)", 500, -0.25, 0.25, 500, -0.25, 0.25 );
  hitmap_trks_diam_minus_ = fs->make<TH2D>( "hitmap_trks_diam_minus", "Proton hits at first diamond plane (z < 0, extrapolated from strips tracks);x (m);y (m)", 500, -0.25, 0.25, 500, -0.25, 0.25 );
  hitmap_trks_diam_plus_matchst0_ = fs->make<TH2D>( "hitmap_trks_diam_plus_matchst0", "Proton hits at first diamond plane (z > 0, extrapolated from strips tracks, with hit candidate in diamonds' plane 0);x (m);y (m)", 500, -0.25, 0.25, 500, -0.25, 0.25 );
  hitmap_trks_diam_minus_matchst0_ = fs->make<TH2D>( "hitmap_trks_diam_minus_matchst0", "Proton hits at first diamond plane (z < 0, extrapolated from strips tracks, with hit candidate in diamonds' plane 0);x (m);y (m)", 500, -0.25, 0.25, 500, -0.25, 0.25 );
}


TreeProducer::~TreeProducer()
{
}

void
TreeProducer::clearTree()
{
  // silicon strips leaves
  strips_tracks_detid_.clear();
  strips_tracks_x0_.clear(); strips_tracks_y0_.clear(); strips_tracks_z0_.clear();
  strips_tracks_sigx0_.clear(); strips_tracks_sigy0_.clear();
  strips_tracks_tx_.clear(); strips_tracks_ty_.clear();
  strips_tracks_sigtx_.clear(); strips_tracks_sigty_.clear();
  strips_tracks_xdiam_.clear(); strips_tracks_ydiam_.clear();

  // diamonds leaves
  //--- digis
  diam_digis_detid_.clear(); diam_digis_arm_.clear(); diam_digis_plane_.clear(); diam_digis_channelid_.clear();
  diam_digis_multiplehits_.clear();
  diam_digis_leadedge_.clear(); diam_digis_trailedge_.clear(); diam_digis_thresvolt_.clear();
  //--- rechits
  diam_rh_side_.clear(); diam_rh_plane_.clear();
  diam_rh_x_.clear(); diam_rh_y_.clear();
  diam_rh_x_size_.clear(); diam_rh_y_size_.clear();
}

// ------------ method called for each event  ------------
void
TreeProducer::analyze( const edm::Event& iEvent, const edm::EventSetup& iSetup )
{
  clearTree();

  //----- diamonds digis
  edm::Handle< edm::DetSetVector<CTPPSDiamondDigi> > diamondDigiColl;
  iEvent.getByToken( diamondDigiToken_, diamondDigiColl );

  bool has_matching_minus = false, has_matching_plus = false;

  for ( edm::DetSetVector<CTPPSDiamondDigi>::const_iterator diads=diamondDigiColl->begin(); diads!=diamondDigiColl->end(); diads++ ) {
    const CTPPSDiamondDetId detid( diads->detId() );
    for ( edm::DetSet<CTPPSDiamondDigi>::const_iterator diadigi=diads->begin(); diadigi!=diads->end(); diadigi++ ) {
      //--- detid-related information
      diam_digis_detid_.push_back( detid() );
      diam_digis_arm_.push_back( detid.arm() ); // 0 = 4-5 ; 1 = 5-6
      diam_digis_plane_.push_back( detid.plane() );
      diam_digis_channelid_.push_back( detid.det() );
      //--- hit-related information
      diam_digis_leadedge_.push_back( diadigi->getLeadingEdge() );
      diam_digis_trailedge_.push_back( diadigi->getTrailingEdge() );
      diam_digis_thresvolt_.push_back( diadigi->getThresholdVoltage() );
      diam_digis_multiplehits_.push_back( diadigi->getMultipleHit() );

      //if ( detid.plane()==0 and diadigi->getTrailingEdge()!=0 ) {
      if ( detid.plane()==0 and ( diadigi->getTrailingEdge()>=1024 and diadigi->getTrailingEdge()<=2253 ) ) { // [ 25.0, 55.0 ] ns
        if ( detid.arm()==1 ) has_matching_plus = true;
        else                  has_matching_minus = true;
      }
    }
  }

  //----- diamonds rechits
  edm::Handle< edm::DetSetVector<CTPPSDiamondRecHit> > diamondRecHitsColl;
  iEvent.getByToken( diamondRecHitToken_, diamondRecHitsColl );

  for ( edm::DetSetVector<CTPPSDiamondRecHit>::const_iterator rhds=diamondRecHitsColl->begin(); rhds!=diamondRecHitsColl->end(); rhds++ ) {
    const CTPPSDiamondDetId detid( rhds->detId() );
    for ( edm::DetSet<CTPPSDiamondRecHit>::const_iterator rh=rhds->begin(); rh!=rhds->end(); rh++ ) {
      diam_rh_side_.push_back( detid.arm() ); // 0 = 45 ; 1 = 56
      diam_rh_plane_.push_back( detid.plane() );
      diam_rh_x_.push_back( rh->getX() );
      diam_rh_y_.push_back( rh->getY() );
      diam_rh_x_size_.push_back( rh->getXWidth() );
      diam_rh_y_size_.push_back( rh->getYWidth() );
    }
  }

  //----- strips local tracks
  edm::Handle< edm::DetSetVector<TotemRPLocalTrack> > stripsColl;
  iEvent.getByToken( stripsLocalTrackToken_, stripsColl );

  for ( edm::DetSetVector<TotemRPLocalTrack>::const_iterator stripds=stripsColl->begin(); stripds!=stripsColl->end(); stripds++ ) {
    const unsigned short detid = stripds->detId();
    for ( edm::DetSet<TotemRPLocalTrack>::const_iterator striplt=stripds->begin(); striplt!=stripds->end(); striplt++ ) {
      if ( !striplt->isValid() ) continue;

      float x_diam, y_diam;
      extrapolateToDiamonds( *striplt, &x_diam, &y_diam );

      strips_tracks_detid_.push_back( detid );
      strips_tracks_x0_.push_back( striplt->getX0()/1.e3 );
      strips_tracks_sigx0_.push_back( striplt->getX0Sigma()/1.e3 );
      strips_tracks_y0_.push_back( striplt->getY0()/1.e3 );
      strips_tracks_sigy0_.push_back( striplt->getY0Sigma()/1.e3 );
      strips_tracks_z0_.push_back( striplt->getZ0()/1.e3 );
      strips_tracks_tx_.push_back( striplt->getTx()/1.e3 );
      strips_tracks_sigtx_.push_back( striplt->getTxSigma()/1.e3 );
      strips_tracks_ty_.push_back( striplt->getTy()/1.e3 );
      strips_tracks_sigty_.push_back( striplt->getTySigma()/1.e3 );
      strips_tracks_xdiam_.push_back( x_diam );
      strips_tracks_ydiam_.push_back( y_diam );

      if ( striplt->getZ0()>0 ) {
        hitmap_trks_diam_plus_->Fill( x_diam, y_diam );
        if ( has_matching_plus ) hitmap_trks_diam_plus_matchst0_->Fill( x_diam, y_diam );
      }
      else {
        hitmap_trks_diam_minus_->Fill( x_diam, y_diam );
        if ( has_matching_minus ) hitmap_trks_diam_minus_matchst0_->Fill( x_diam, y_diam );
      }

    }
  }

  tree_->Fill();
}

void
TreeProducer::extrapolateToDiamonds( const TotemRPLocalTrack& lt, float* x, float* y ) const
{
  *x = *y = 0.;
  if ( !lt.isValid() ) return;

  const float z_diam = ( lt.getZ0()>0. ) ? 215.5e3 : -215.5e3;

  *x = ( lt.getX0()+( z_diam-lt.getZ0() )*lt.getTx() )/1.e3;
  *y = ( lt.getY0()+( z_diam-lt.getZ0() )*lt.getTy() )/1.e3;
}

// ------------ method called once each job just before starting event loop  ------------
void 
TreeProducer::beginJob()
{
  tree_->Branch( "strips_track_detid", &strips_tracks_detid_ );
  tree_->Branch( "strips_track_x0", &strips_tracks_x0_ );
  tree_->Branch( "strips_track_y0", &strips_tracks_y0_ );
  tree_->Branch( "strips_track_z0", &strips_tracks_z0_ );
  tree_->Branch( "strips_track_x0_sig", &strips_tracks_sigx0_ );
  tree_->Branch( "strips_track_y0_sig", &strips_tracks_sigy0_ );
  tree_->Branch( "strips_track_Tx", &strips_tracks_tx_ );
  tree_->Branch( "strips_track_Ty", &strips_tracks_ty_ );
  tree_->Branch( "strips_track_Tx_sig", &strips_tracks_sigtx_ );
  tree_->Branch( "strips_track_Ty_sig", &strips_tracks_sigty_ );
  tree_->Branch( "strips_track_x_diam", &strips_tracks_xdiam_ );
  tree_->Branch( "strips_track_y_diam", &strips_tracks_ydiam_ );

  tree_->Branch( "diamonds_digi_detid", &diam_digis_detid_ );
  tree_->Branch( "diamonds_digi_arm", &diam_digis_arm_ );
  tree_->Branch( "diamonds_digi_plane", &diam_digis_plane_ );
  tree_->Branch( "diamonds_digi_channelid", &diam_digis_channelid_ );
  tree_->Branch( "diamonds_digi_leading_edge", &diam_digis_leadedge_ );
  tree_->Branch( "diamonds_digi_trailing_edge", &diam_digis_trailedge_ );
  tree_->Branch( "diamonds_digi_threshold_voltage", &diam_digis_thresvolt_ );
  tree_->Branch( "diamonds_digi_multiple_hits", &diam_digis_multiplehits_ );

  tree_->Branch( "diamonds_rh_side", &diam_rh_side_ );
  tree_->Branch( "diamonds_rh_plane", &diam_rh_plane_ );
  tree_->Branch( "diamonds_rh_x", &diam_rh_x_ );
  tree_->Branch( "diamonds_rh_y", &diam_rh_y_ );
  tree_->Branch( "diamonds_rh_x_size", &diam_rh_x_size_ );
  tree_->Branch( "diamonds_rh_y_size", &diam_rh_y_size_ );
}

// ------------ method called once each job just after ending the event loop  ------------
void 
TreeProducer::endJob() 
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
TreeProducer::fillDescriptions( edm::ConfigurationDescriptions& descriptions ) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault( desc );
}

//define this as a plug-in
DEFINE_FWK_MODULE( TreeProducer );
