// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAINPARAMS_H
#define BITCOIN_CHAINPARAMS_H

#include <chainparamsbase.h>
#include <consensus/params.h>
#include <primitives/block.h>
#include <chain.h>
#include <protocol.h>
#include <validation.h>

#include <memory>
#include <vector>

static const uint32_t CHAIN_NO_GENESIS = 444444;
static const uint32_t CHAIN_NO_STEALTH_SPEND = 444445; // used hardened

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

typedef std::map<int, uint256> MapCheckpoints;

struct CCheckpointData {
    MapCheckpoints mapCheckpoints;

    int GetHeight() const {
        const auto& final_checkpoint = mapCheckpoints.rbegin();
        return final_checkpoint->first /* height */;
    }
};

/**
 * Holds various statistics on transactions within a chain. Used to estimate
 * verification progress during chain sync.
 *
 * See also: CChainParams::TxData, GuessVerificationProgress.
 */
struct ChainTxData {
    int64_t nTime;    //!< UNIX timestamp of last known number of transactions
    int64_t nTxCount; //!< total number of transactions between genesis and that timestamp
    double dTxRate;   //!< estimated number of transactions per second after that timestamp
};

class CImportedCoinbaseTxn
{
public:
    CImportedCoinbaseTxn(uint32_t nHeightIn, uint256 hashIn) : nHeight(nHeightIn), hash(hashIn) {};
    uint32_t nHeight;
    uint256 hash; // hash of output data
};

class TreasuryFundSettings
{
public:
    TreasuryFundSettings(std::string sAddrTo, int nMinTreasuryStakePercent_, int nTreasuryOutputPeriod_)
        : sTreasuryFundAddresses(sAddrTo), nMinTreasuryStakePercent(nMinTreasuryStakePercent_), nTreasuryOutputPeriod(nTreasuryOutputPeriod_) {};

    std::string sTreasuryFundAddresses;
    int nMinTreasuryStakePercent; // [0, 100]
    int nTreasuryOutputPeriod; // dev fund output is created every n blocks
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Bitcoin system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,
        EXT_PUBLIC_KEY,
        EXT_SECRET_KEY,
        STEALTH_ADDRESS,
        EXT_KEY_HASH,
        EXT_ACC_HASH,
        EXT_PUBLIC_KEY_BTC,
        EXT_SECRET_KEY_BTC,
        PUBKEY_ADDRESS_256,
        SCRIPT_ADDRESS_256,
        STAKE_ONLY_PKADDR,
        MAX_BASE58_TYPES
    };

    const Consensus::Params& GetConsensus() const { return consensus; }
    const CMessageHeader::MessageStartChars& MessageStart() const { return pchMessageStart; }
    int GetDefaultPort() const { return nDefaultPort; }

    int BIP44ID(bool fLegacy) const { return fLegacy ? nBIP44IDLegacy : nBIP44IDCurrent; }

    uint32_t GetModifierInterval() const { return nModifierInterval; }
    uint32_t GetStakeMinConfirmations() const { return nStakeMinConfirmations; }
    uint32_t GetTargetSpacing() const { return nTargetSpacing; }
    uint32_t GetTargetTimespan() const { return nTargetTimespan; }

    uint32_t GetStakeTimestampMask(int nHeight) const { return nStakeTimestampMask; }
    int64_t GetCoinYearReward(int64_t nTime) const;

    int64_t GetBaseBlockReward() const;
    int GetCoinYearPercent(int year) const;
    const TreasuryFundSettings *GetTreasuryFundSettings(int nHeight) const;
    bool PushTreasuryFundSettings(int64_t time_from, TreasuryFundSettings &settings);
    const std::vector<std::pair<int64_t, TreasuryFundSettings> > &GetTreasuryFundSettings() const { return vTreasuryFundSettings; };

    CAmount GetProofOfStakeReward(const CBlockIndex *pindexPrev, int64_t nFees) const;
    CAmount GetProofOfStakeRewardAtYear(int year) const;
    CAmount GetProofOfStakeRewardAtHeight(int nHeight) const;
    int64_t GetMaxSmsgFeeRateDelta(int64_t smsg_fee_prev, int64_t time) const;

    bool CheckImportCoinbase(int nHeight, uint256 &hash) const;
    uint32_t GetLastImportHeight() const { return nLastImportHeight; }

    const CBlock& GenesisBlock() const { return genesis; }
    /** Default value for -checkmempool and -checkblockindex argument */
    bool DefaultConsistencyChecks() const { return fDefaultConsistencyChecks; }
    /** Policy: Filter transactions that do not match well-defined patterns */
    bool RequireStandard() const { return fRequireStandard; }
    /** If this chain is exclusively used for testing */
    bool IsTestChain() const { return m_is_test_chain; }
    /** If this chain allows time to be mocked */
    bool IsMockableChain() const { return m_is_mockable_chain; }
    uint64_t PruneAfterHeight() const { return nPruneAfterHeight; }
    /** Minimum free space (in GB) needed for data directory */
    uint64_t AssumedBlockchainSize() const { return m_assumed_blockchain_size; }
    /** Minimum free space (in GB) needed for data directory when pruned; Does not include prune target*/
    uint64_t AssumedChainStateSize() const { return m_assumed_chain_state_size; }
    /** Whether it is possible to mine blocks on demand (no retargeting) */
    bool MineBlocksOnDemand() const { return consensus.fPowNoRetargeting; }
    /** Return the network string */
    std::string NetworkIDString() const { return strNetworkID; }
    /** Return the list of hostnames to look up for DNS seeds */
    const std::vector<std::string>& DNSSeeds() const { return vSeeds; }
    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    const std::vector<unsigned char>& Bech32Prefix(Base58Type type) const { return bech32Prefixes[type]; }
    const std::string& Bech32HRP() const { return bech32_hrp; }
    const std::vector<SeedSpec6>& FixedSeeds() const { return vFixedSeeds; }
    const CCheckpointData& Checkpoints() const { return checkpointData; }
    const ChainTxData& TxData() const { return chainTxData; }

    bool IsBech32Prefix(const std::vector<unsigned char> &vchPrefixIn) const;
    bool IsBech32Prefix(const std::vector<unsigned char> &vchPrefixIn, CChainParams::Base58Type &rtype) const;
    bool IsBech32Prefix(const char *ps, size_t slen, CChainParams::Base58Type &rtype) const;

    std::string NetworkID() const { return strNetworkID; }

    void SetCoinYearReward(int64_t nCoinYearReward_)
    {
        assert(strNetworkID == "regtest");
        nCoinYearReward = nCoinYearReward_;
    }
    Consensus::Params& GetConsensus_nc() { assert(strNetworkID == "regtest"); return consensus; }

    void SetBlockReward(int64_t nBlockReward_)
    {
        assert(strNetworkID == "regtest");
        nBlockReward = nBlockReward_;
    }

    void SetAnonRestricted(bool bFlag) {
        anonRestricted = bFlag;
    }

    bool IsAnonRestricted() const {
        return anonRestricted;
    }

    std::string GetRecoveryAddress() const {
        return anonRecoveryAddress;
    }

    void SetRecoveryAddress(const std::string& addr) {
        anonRecoveryAddress = addr;
    }

    void SetAnonMaxOutputSize(uint32_t size){
        anonMaxOutputSize = size;
    }

    uint32_t GetAnonMaxOutputSize() const {
        return anonMaxOutputSize;
    }

    std::set<std::uint64_t> GetBlacklistedAnonOutputs() {
        return blacklistedAnonTxs;
    }

    bool IsBlacklistedAnonOutput(std::uint64_t index) const {
        return blacklistedAnonTxs.count(index);
    }

    void SetBlacklistedAnonOutput(const std::set<std::uint64_t>& anonIndexes){
        blacklistedAnonTxs = anonIndexes;
    }

protected:
    CChainParams() {}

    Consensus::Params consensus;
    CMessageHeader::MessageStartChars pchMessageStart;
    int nDefaultPort;
    int nBIP44IDLegacy;
    int nBIP44IDCurrent;

    uint32_t nModifierInterval;         // seconds to elapse before new modifier is computed
    uint32_t nStakeMinConfirmations;    // min depth in chain before staked output is spendable
    uint32_t nTargetSpacing;            // targeted number of seconds between blocks
    uint32_t nTargetTimespan;
    CAmount nBlockReward;               // Block reward for PoS blocks,static
    CAmount nBlockRewardIncrease;               // Block reward for PoS blocks,static
    uint32_t nStakeTimestampMask = (1 << 4) - 1; // 4 bits, every kernel stake hash will change every 16 seconds
    int64_t nCoinYearReward = 2 * CENT; // 2% per year, See GetCoinYearReward

    std::array<int, 47> nBlockPerc; //reward percentage each year
    uint32_t nLastImportHeight = 0;       // always 0 on ghost
    bool anonRestricted = DEFAULT_ANON_RESTRICTED;
    std::string anonRecoveryAddress;
    std::uint32_t anonMaxOutputSize = 2;

    std::vector<std::pair<int64_t, TreasuryFundSettings> > vTreasuryFundSettings;


    uint64_t nPruneAfterHeight;
    uint64_t m_assumed_blockchain_size;
    uint64_t m_assumed_chain_state_size;
    std::vector<std::string> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    std::vector<unsigned char> bech32Prefixes[MAX_BASE58_TYPES];
    std::string bech32_hrp;
    std::string strNetworkID;
    CBlock genesis;
    std::vector<SeedSpec6> vFixedSeeds;
    bool fDefaultConsistencyChecks;
    bool fRequireStandard;
    bool m_is_test_chain;
    bool m_is_mockable_chain;
    CCheckpointData checkpointData;
    ChainTxData chainTxData;
    std::set<std::uint64_t> blacklistedAnonTxs;
};

/**
 * Creates and returns a std::unique_ptr<CChainParams> of the chosen chain.
 * @returns a CChainParams* of the chosen chain.
 * @throws a std::runtime_error if the chain is not supported.
 */
std::unique_ptr<CChainParams> CreateChainParams(const ArgsManager& args, const std::string& chain);

/**
 * Return the currently selected parameters. This won't change after app
 * startup, except for unit tests.
 */
const CChainParams &Params();
const CChainParams *pParams();

/**
 * Sets the params returned by Params() to those for the given chain name.
 * @throws std::runtime_error when the chain is not supported.
 */
void SelectParams(const std::string& chain);

/**
 * Toggle old parameters for unit tests
 */
void SetOldParams(std::unique_ptr<CChainParams> &params);
void ResetParams(std::string sNetworkId, bool fParticlModeIn);

/**
 * mutable handle to regtest params
 */
CChainParams &RegtestParams();

std::set<std::uint64_t> GetAnonIndexFromString(const std::string& str);

#endif // BITCOIN_CHAINPARAMS_H
