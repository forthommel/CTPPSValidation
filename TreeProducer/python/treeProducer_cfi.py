import FWCore.ParameterSet.Config as cms

ctppsTreeProducer = cms.EDAnalyzer('TreeProducer',
    stripsLocalTrackLabel = cms.InputTag('totemRPLocalTrackFitter'),
    diamondDigiLabel = cms.InputTag('ctppsDiamondRawToDigi', 'TimingDiamond'),
    diamondRecHitLabel = cms.InputTag('ctppsDiamondRecHits'),
)
