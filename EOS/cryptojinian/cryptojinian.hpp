/**
 *  @dev deaso, yukiexe
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <algorithm>

#include "config.hpp"
#include "utils.hpp"
#include "kyubey.hpp"
#include "dividend.hpp"

using namespace eosio ;
using namespace config ;
using namespace kyubeytool ;

class [[eosio::contract]] cryptojinian : public eosio::contract {
    public:
        cryptojinian( name receiver, name code, datastream<const char*> ds ) :
        contract( receiver, code, ds ),
        _contract_kyubey( receiver, code, ds ),
        _contract_dividend(receiver, code, ds ),
        _global(receiver, receiver.value),
        _coins(receiver, receiver.value),
        _players(receiver, receiver.value),
        _usedcoins(receiver, receiver.value) {}

        TABLE accounts : kyubey::account {};
        TABLE stat : kyubey::currency_stats {};
        struct [[eosio::table("dividend")]] st_dividend : kyubeytool::dividend::st_d_global {};
        TABLE playerinfo : kyubeytool::dividend::st_player_info {};
    
        struct [[eosio::table("global")]] st_global {
            uint64_t id = 0;
            capi_checksum256 hash; // hash of the game seed, 0 when idle.
            // std::map<uint64_t, uint64_t> usedcoins; // 1 uint64_t for 64 coins
            // std::map<uint64_t, uint64_t> usedspilt64; // for faster finding
            // std::map<uint64_t, uint64_t> usedspilt6400; // for faster finding
            //example(for uint8_t):
            //coinnumbers: [8,13,16,29,31,32]
            //usedcoins: [1,9,0,11] ([00000001.00001001,00000000,00001011])
            //remainspilt8: [1,2,0,3]
            //remainspilt16: [3,3]
            // std::map<uint64_t, uint64_t> typecounts;
            uint64_t remainamount; // return remain coin amounts

            const asset miningcost() const { return cost_table( remainamount ); }

            auto primary_key() const { return id; }
            EOSLIB_SERIALIZE(st_global, (id)(hash)(remainamount)) 
        };

        TABLE player {
            capi_name playername;
            capi_checksum256 seed;
            vector<uint64_t> coins; // coins, for id
            capi_name sponsor = ( DEF_SPONSOR ).value ;
            vector<capi_name> refs ;

            auto primary_key() const { return playername; }
            EOSLIB_SERIALIZE(player, (playername)(seed)(coins)(sponsor)(refs) )
        };

        TABLE coin {
            uint64_t id;
            capi_name owner;
            uint64_t type; // type :xxyy, xx for valuetype, yy for cointype
            // BTC 1 cointype, ETH 2 cointype
            // for example: 2 valuetype BTC: 201
            uint64_t number;

            static uint64_t str_to_coin_type( string &str ) {
                return string_to_int( str ) ;
            }

            auto primary_key() const { return id; }
            EOSLIB_SERIALIZE(coin, (id)(owner)(type)(number))
        };
        
        struct [[eosio::table("frozencoins")]] st_frozen_coin {
            uint64_t id;
            uint32_t time_limit = 0;
            auto primary_key() const { return id; }
            EOSLIB_SERIALIZE(st_frozen_coin, (id)(time_limit))
        };

        /*
            // << 16 to fix usedspilt64;
            // << 32 to fix usedspilt6400;
            // << 48 to fix typecounts;
        */
        TABLE usedcoins {
            uint64_t key = 0;
            uint64_t value = 0;

            auto primary_key() const { return key; }
            EOSLIB_SERIALIZE(usedcoins, (key)(value)) 
        };

        TABLE st_miningqueue {
            uint64_t id ;
            capi_name miner ;

            auto primary_key() const { return id; }
            EOSLIB_SERIALIZE(st_miningqueue, (id)(miner))
        };

        TABLE order {
            uint64_t id;
            capi_name account;
            asset bid;
            vector<uint64_t> the_coins_for_sell ; // coins, for id
            uint64_t timestamp;

            auto primary_key() const { return id; }
            // auto get_unit_price() const { return bid / the_coins_for_sell.number  }
            EOSLIB_SERIALIZE(order, (id)(account)(bid)(the_coins_for_sell)(timestamp))
        };

        struct [[eosio::table("collection")]] st_collection {
            vector<uint64_t> records ;
        };
        
        struct [[eosio::table("collcd")]] st_collection_cd {
            vector<uint32_t> time_limit ;
        };

        struct [[eosio::table("buybackqueue")]]  st_buybackqueue {
            asset limit ;
            asset price ;
        };

        typedef singleton<"global"_n, st_global> singleton_global_t;
        typedef eosio::multi_index<"player"_n, player> player_t;
        typedef eosio::multi_index<"coin"_n, coin> coin_t;
        typedef eosio::multi_index<"usedcoins"_n, usedcoins> usedcoins_t;
        typedef eosio::multi_index<"miningqueue"_n, st_miningqueue> miningqueue_t;
        typedef eosio::multi_index<"order"_n, order> order_t;
        typedef eosio::multi_index<"frozencoins"_n,  st_frozen_coin> frozencoins_t;
        typedef singleton<"collection"_n, st_collection> collection_t;
        typedef singleton<"collcd"_n, st_collection_cd> singleton_collcd_t;
        typedef singleton<"buybackqueue"_n, st_buybackqueue> singleton_buybackqueue_t;

        singleton_global_t _global;
        player_t _players;
        coin_t _coins; 
        usedcoins_t _usedcoins;
        kyubey _contract_kyubey ;
        dividend _contract_dividend;

        struct st_rec_takeOrder {
            order matched_order ;
            name buyer ; 
            string message = "Order matched." ;
        };
        
    private:
        void setcoin(const name &owner, const uint64_t &type, const uint64_t &number);
        void deletecoin(const uint64_t &id);
        uint64_t addcoincount( uint64_t type );
        uint64_t findcoinpos( uint32_t &input );
        void newcoinbypos(const name owner, const uint64_t pos);
        void exchangecoin(const name &owner, const uint64_t &id);
        inline uint32_t toType( const uint32_t &x, const uint32_t &y) {
            return x * 100 + y + 1;
        }
        bool cd_check( const uint64_t &id );

        void update_frozen_time_limit( const name &owner, const uint32_t &type, const uint64_t &quantity, const uint32_t &frozen_days );
        // void update_frozen_time_limit( const name &owner, const uint32_t &type, const uint32_t &frozen_days );
        void SplitString(const std::string& s, vector<uint64_t>& v, const std::string& c);

        auto join_game_processing(const name &account) {
            auto player = _players.find(account.value);
            if (player == _players.end()) { // new
                player = _players.emplace(get_self(), [&](auto &p) {
                    p.playername = account.value;
                    p.sponsor = DEF_SPONSOR.value;
                });
            }
            return player;
        }

        void token_mining( name miner, asset quantity, string memo );
        void token_mining_with_stake(name miner, asset quantity, string memo);
        inline const asset fee_processing( asset &quantity ) ;

        void collection_counter(const name &owner, const uint32_t &yy, uint64_t &r) {
            auto player = _players.require_find(owner.value, "Player not found.");
            // type :xxyy, xx for valuetype, yy for cointype
            // BTC 1 cointype, ETH 2 cointype
            // for example: 2 valuetype BTC: 201

            vector<decltype(_coins.begin())> coins;
            for ( const auto &cid : player->coins ) {
                coins.push_back(_coins.find(cid));
            }

            vector<vector<uint64_t>> counter(_coinvalues.size());
            counter[yy] = vector<uint64_t>(_coinvalues[yy].size(), 0);
            for ( uint32_t xx = 0 ; xx < counter[yy].size(); ++xx ) {
                for ( const auto &itr : coins ) {
                    if ( itr->type == toType(xx, yy) ) {
                        counter[yy][xx]++;
                    }
                }
                if ( xx == 0 ) r = counter[yy][xx];
                if ( r > counter[yy][xx] ) r = counter[yy][xx];
            }
        }

        inline auto toCollTypes(const uint32_t &typeofcoin) {
            const uint32_t &type = (typeofcoin % 100) -1;
            vector<uint32_t> collTypes{1, type};
            collTypes.emplace_back(28);
            switch( type ) {
                case 0 :
                    collTypes.emplace_back(22);
                    break;
                case 1 :
                    collTypes.emplace_back(23);
                    break;
                case 2 :
                    collTypes.emplace_back(22);
                    break;
                case 3 :
                    collTypes.emplace_back(24);
                    break;
                case 4 :
                    collTypes.emplace_back(27);
                    break;
                case 5 :
                    collTypes.emplace_back(25);
                    break;
                case 6 :
                    collTypes.emplace_back(23);
                    break;
                case 7 :
                    collTypes.emplace_back(23);
                    break;
                case 8 :
                    collTypes.emplace_back(26);
                    break;
                case 9 :
                    collTypes.emplace_back(26);
                    break;
                case 10 :
                    collTypes.emplace_back(22);
                    break;
                case 11 :
                    collTypes.emplace_back(23);
                    break;
                case 12 :
                    collTypes.emplace_back(23);
                    break;
                case 13 :
                    collTypes.emplace_back(24);
                    break;
                case 14 :
                    collTypes.emplace_back(23);
                    break;
                case 15 :
                    collTypes.emplace_back(23);
                    break;
                case 16 :
                    collTypes.emplace_back(24);
                    break;
                case 17 :
                    collTypes.emplace_back(25);
                    break;
                case 18 :
                    collTypes.emplace_back(23);
                    break;
                case 19 :
                    collTypes.emplace_back(27);
                    break;
                case 20 :
                    collTypes.emplace_back(25);
                    break;
                case 21 :
                    collTypes.emplace_back(23);
                    break;
            }
            return collTypes;
        }

        inline void collection_checker( uint64_t &r, const vector<uint64_t> &v ) {
            for ( uint32_t xx = 0 ; xx < v.size(); xx++ )
                if ( r > v[xx] ) r = v[xx] ;
        }

        void onTransfer(name from, name to, asset quantity, string memo);
        // onTransfer() ->
        void join_miningqueue( const name &miner, const asset &totalcost );
        void ref_processing(const name &miner, const name &sponsor = DEF_SPONSOR );

        inline void token_unstake_and_burn(name owner, asset quantity, string memo) {
            _contract_dividend.unstake(owner, quantity, memo);
            _contract_kyubey.burn(owner, quantity, memo);
        }

        inline vector<uint32_t> merge_seed(const capi_checksum256 &s);

    public:
        // Contract management
        ACTION init();
        ACTION clear();

        // Token management
        ACTION issue( name to, asset quantity, string memo ){
            _contract_kyubey.issue(to, quantity, memo);
            _contract_dividend.stake(to, quantity);
        }
        ACTION burn( name to, asset quantity, string memo ){
            require_auth(get_self());
            token_unstake_and_burn(to, quantity, memo);
        }
        ACTION transfer(name from, name to, asset quantity, string memo) {
            require_auth(from);
            _contract_kyubey.transfer(from, to, quantity, memo);
            _contract_dividend.unstake(from, quantity, memo);
            _contract_dividend.stake(to, quantity);
        }

        // Coin management
        ACTION ownersetcoin(const name &owner, const uint64_t &type, const uint64_t &number) {
            require_auth(_self);
            setcoin(owner, type, number);
        }        
        ACTION ownerdelcoin(const uint64_t &id) {
            require_auth(_self);
            deletecoin(id);
        }
        ACTION exchange(const string &inputs);
        ACTION exchangedown(const uint64_t inputid, const uint64_t goal);
        ACTION unfreezecoin(const uint64_t &id); 

        // Game management
        ACTION mining( const capi_checksum256 &seed ) {
            require_auth(_self);

            miningqueue_t _miningqueue(_self, _self.value);
            auto itr = _miningqueue.begin();
            if ( itr == _miningqueue.end() ) return ;

            const auto &mc = _global.get().miningcost();
            auto v_seed = merge_seed( seed ) ;
            uint32_t n = 0 ;
            name miner ;
            while( itr != _miningqueue.end() && n != v_seed.size() ) {
                miner = name(itr->miner) ;
                newcoinbypos(miner, findcoinpos(v_seed[n])) ;

                SEND_INLINE_ACTION(*this, recmining, { _self, "active"_n }, { miner });
                _miningqueue.erase(itr);
                
                itr = _miningqueue.begin();
                n++ ;
                if ( n > config::MINING_TIMES ) return ;
            }
        }

        // Trade management
        /*
         * straddorder = type_order type_coin n_coin
         * type_order: 1 or 2
         * type_coin:  coin's type
         * n_coin:     coin's number
        */
        ACTION pushorder( const name &owner, asset &eos, string &straddorder ) {
            require_auth(owner);

            auto player = join_game_processing(owner);

            auto v_str = explode(straddorder, ' ');
            eosio_assert(v_str.size() == 3, "Error memo");

            uint32_t type_order = string_to_int( v_str[0] ) ;
            auto type_coin = coin::str_to_coin_type( v_str[1] ) ;
            auto n_coin = string_to_int( v_str[2] ) ;
            eosio_assert(n_coin != 0, "amount of coin can't be 0.");

            // coll
            collection_t collection(_self, owner.value);
            const auto coll = collection.get_or_create(_self, st_collection { .records = vector<uint64_t> (22 + 6 + 1,0) } );

            uint64_t amountOfFrozenCoin = 0;
 
            // collTypes
            for (const auto &type : toCollTypes(type_coin)) {
                amountOfFrozenCoin += coll.records[type];
            }

            frozencoins_t frozencoins(_self, owner.value);
            auto coin = _coins.begin();
            vector<uint64_t> pcoins;
            vector<decltype(coin)> itr_coins;
            for ( const auto &id : player->coins ) {
                coin = _coins.find(id);
                if ( coin->owner != ("eosio.token"_n).value /* not on order */
                     && coin->type == type_coin ) {
                    auto itr = frozencoins.find(id);
                    if ( amountOfFrozenCoin > 0 ) {
                        --amountOfFrozenCoin;
                        if (itr != frozencoins.end()) {
                            pcoins.push_back(id);
                            itr_coins.push_back(coin);
                        }
                    }
                    else {
                        pcoins.push_back(id);
                        itr_coins.push_back(coin);
                    }

                    if ( pcoins.size() == n_coin) break;
                }
            }
            eosio_assert(pcoins.size() == n_coin, "Player dont have enough coins for sell order");

            // transfer coin 所有权 to "eosio.token"_n
            for (const auto &coin : itr_coins) {
                _coins.modify(coin, _self, [&](auto &c) {
                    c.owner = ("eosio.token"_n).value;
                });
            }

            // add TRADE_FEE
            eos.set_amount(eos.amount * (1 + TRADE_FEE));

            // push order
            order_t _orders(_self, _self.value);
            uint64_t neworderid = type_order * 1000000;
            auto getorder = _orders.find(neworderid);
            while(getorder != _orders.end()){
                neworderid ++;
                getorder = _orders.find(neworderid);
            }
            _orders.emplace(_self, [&](auto &o) {
                o.id = neworderid;
                o.account = owner.value;
                o.bid = eos;
                o.the_coins_for_sell = pcoins; // set coins
                o.timestamp = current_time();
            });
        }
        ACTION cancelorder( const name &account, const uint64_t &order_id ) {
            require_auth(account);

            order_t _orders( get_self(), get_self().value );
            auto itr = _orders.require_find(order_id, "Trade id is not found" );
            eosio_assert(itr->account == account.value, "Account does not match");
            
            // transfer coin 所有权 back to seller
            auto citr = _coins.begin() ;
            for (auto cid : itr->the_coins_for_sell) {
                citr = _coins.find(cid);
                _coins.modify(citr, get_self(), [&](auto &c) {
                    c.owner = itr->account;
                });
            }

            _orders.erase(itr);
        }
        ACTION syscxlorder( const uint64_t &order_id ) {
            require_auth(_self);
            order_t _orders(_self, _self.value);
            auto itr = _orders.require_find(order_id, "Trade id is not found" );

            // transfer coin 所有权 back to seller
            auto citr = _coins.begin() ;
            for (auto cid : itr->the_coins_for_sell) {
                citr = _coins.find(cid);
                _coins.modify(citr, get_self(), [&](auto &c) {
                    c.owner = itr->account;
                });
            }
            
            _orders.erase(itr);
        }
        void takeorder( const name &buyer, const uint64_t &order_id, asset &eos );

        // Dividend management
        ACTION claim( name &from ) {
            require_auth(get_self());
            if ( from == _self ) return;
            _contract_dividend.claim( from, _contract_kyubey.get_balance( from, config::TOKEN_SYMBOL ) );
        }

        // Coll. management
        ACTION collclaim( const name &owner, uint32_t &type ) {
            require_auth(owner);
            if ( owner == _self ) return;
            
            eosio_assert( type <= 22 + 6 + 1, "Type error");
            type --;

            collection_t coll(_self, owner.value);
            auto itr = coll.get_or_create(_self, st_collection { .records = vector<uint64_t> (22 + 6 + 1,0) } );

            uint64_t r ;
            if ( type <= 27 ) {
                const auto &v = collection_combination_parameters(type);
                for(auto&& yy : v) {
                    collection_counter(owner, yy, r);
                }
            } else if ( type == 28 ) {
                for (uint32_t i = 0 ; i < 22 ; ++i) {
                    if ( i == 0 ) r = itr.records[i];
                    if ( r > itr.records[i] ) r = itr.records[i];
                }
            }

            eosio_assert(r > itr.records[type], "Not Enough Coin");
            if ( type != 28 )
                token_mining_with_stake(owner, config::bouns_table(type), string{"Bouns from collection claim."});
            else
                _contract_dividend.collection_claim(owner); 
            
            SEND_INLINE_ACTION( *this, reccollclaim, { _self, "active"_n }, { owner, type } );
            itr.records[type] ++;
            coll.set(itr, _self);
        }

        // Buyback management
        ACTION joinbuybackq( const name &buyer, const asset &quantity ) {
            require_auth(buyer);
            if ( buyer == _self ) return;
            eosio_assert(quantity.is_valid(), "invalid token transfer");
            eosio_assert(quantity.symbol == config::TOKEN_SYMBOL, "Only CCC token is allowed");
            eosio_assert(quantity.amount > 0, "must transfer a positive amount"); // 正數的結界
            auto buyer_balance = _contract_kyubey.get_balance(buyer, quantity.symbol);
            eosio_assert(buyer_balance >= quantity, "Must have enough token.");
            
            token_unstake_and_burn(buyer, quantity, string{""});

            singleton_buybackqueue_t buybackqueue(_self, buyer.value);
            eosio_assert( ! buybackqueue.exists() , "Entered buybackqueue before." ); 
            buybackqueue.set(st_buybackqueue {.limit = quantity, 
                                                .price = asset(0, EOS_SYMBOL)}, _self);
            // set total
            singleton_buybackqueue_t bbq_self(_self, _self.value);
            auto bbq = bbq_self.get_or_create(_self, st_buybackqueue{.limit = asset(0, TOKEN_SYMBOL),
                                                                     .price = asset(0, EOS_SYMBOL)});
            bbq.limit += quantity ;
            auto _d_g = _contract_dividend._global.get() ;
            uint64_t price = _d_g.earnings_for_buyback * config::PRICE_SCALE / bbq.limit.amount ;
            bbq.price = asset( price, EOS_SYMBOL);
            bbq_self.set(bbq, _self);
        }
        ACTION outbuybackq( const name &buyer ) { // Deprecated
            return; // Deprecated
            require_auth(buyer);
            if ( buyer == _self ) return;
            singleton_buybackqueue_t buybackqueue(_self, buyer.value);
            eosio_assert( buybackqueue.exists(), "Did not entered buybackqueue before." );
            
            // set total
            singleton_buybackqueue_t bbq_self(_self, _self.value);
            auto bbq = bbq_self.get();
            bbq.limit -= buybackqueue.get().limit ;
            if ( bbq.limit == asset(0, TOKEN_SYMBOL) )
                bbq.price = asset(0, EOS_SYMBOL);
            else {
                auto _d_g = _contract_dividend._global.get() ;
                uint64_t price = _d_g.earnings_for_buyback * config::PRICE_SCALE / bbq.limit.amount ;
                bbq.price = asset( price, EOS_SYMBOL);
            }
            bbq_self.set(bbq, _self);
          
            buybackqueue.remove();
        }
        ACTION cleanbbq() {
            require_auth(_self);
            singleton_buybackqueue_t bbq_self(_self, _self.value);
            eosio_assert(bbq_self.exists(), "Did not entered buybackqueue before.");
           
            bbq_self.remove();
            _contract_dividend.cleanbuyback();
        }
        ACTION buyback( const name &buyer, asset &quantity ) {
            require_auth(_self);
            if ( buyer == _self ) return;
            eosio_assert(quantity.is_valid(), "invalid token transfer");
            eosio_assert(quantity.symbol == TOKEN_SYMBOL, "Only CCC token is allowed");
            eosio_assert(quantity.amount > 0, "must transfer a positive amount"); // 正數的結界
            // auto buyer_balance = _contract_kyubey.get_balance( buyer, quantity.symbol );
            // eosio_assert(buyer_balance >= quantity, "Must have enough x.");
            
            singleton_buybackqueue_t buybackqueue(_self, buyer.value);
            eosio_assert( buybackqueue.exists(), "Did not entered buybackqueue before." ); 
            auto bbq_buyer = buybackqueue.get();
            eosio_assert( bbq_buyer.limit >= quantity, "Must have enough token limit.");
            
            singleton_buybackqueue_t bbq_self(_self, _self.value);
            quantity *= bbq_self.get().price.amount ;
            quantity /= config::PRICE_SCALE ;
            asset delta( quantity.amount, EOS_SYMBOL ) ;
            if ( delta.is_valid() && delta.amount > 0) { 
                action(permission_level{ _self, "active"_n},
                    "eosio.token"_n, "transfer"_n,
                    make_tuple( _self, buyer, delta, string("Claim buyback."))
                ).send();

                bbq_buyer.limit -= quantity ;
                buybackqueue.set(bbq_buyer, _self);
    
            }
            buybackqueue.remove();
        }
        ACTION autobuyback( const name &buyer ) {
            require_auth(_self);
            if ( buyer == _self ) return;
            // auto buyer_balance = _contract_kyubey.get_balance(buyer, TOKEN_SYMBOL);
            
            singleton_buybackqueue_t buybackqueue(_self, buyer.value);
            eosio_assert( buybackqueue.exists(), "Did not entered buybackqueue before." ); 
            auto bbq_buyer = buybackqueue.get();
            // eosio_assert( bbq_buyer.limit >= quantity, "Must have enough token limit.");
            auto quantity = bbq_buyer.limit;
            // eosio_assert(buyer_balance >= quantity, "Must have enough token.");

            // total
            singleton_buybackqueue_t bbq_self(_self, _self.value);
            quantity *= bbq_self.get().price.amount ;
            quantity /= config::PRICE_SCALE ;
            asset delta( quantity.amount, EOS_SYMBOL ) ;
            
            if ( delta.is_valid() && delta.amount > 0) { 
                action(permission_level{ _self, "active"_n},
                    "eosio.token"_n, "transfer"_n,
                    make_tuple( _self, buyer, delta, string("Claim buyback."))
                ).send();

                bbq_buyer.limit -= quantity ;

                buybackqueue.set(bbq_buyer, _self);
            }

            buybackqueue.remove();
        }

        // Rec
        ACTION receipt(const st_rec_takeOrder& take_order_record) {}
        ACTION recmining( const name &miner ) {
            require_auth(_self);
        }
        ACTION reccollclaim( const name &account, uint32_t &type ) {}
        ACTION recpcoll( const name &account, vector<uint64_t> number_of_coins ) {}

        ACTION frozen( const name &owner, uint32_t &type) {
            require_auth(_self);
            collection_t coll(_self, owner.value);
            auto itr = coll.get_or_create(_self, st_collection { .records = vector<uint64_t> (22 + 6 + 1,0) } );
            const uint64_t quantity = itr.records[type];
            eosio_assert(quantity != 0, "quantity 0");
            auto itr_p = _players.require_find(owner.value, "Player not found.") ;
            const auto &v_param = collection_combination_parameters(type);
            for (const auto &id : itr_p->coins) {
                for (auto i = 0; i < quantity; ++i) {
                    for(const auto& yy : v_param) {
                        for ( uint8_t xx = 0 ; xx < _coinvalues[yy].size(); ++xx ) {
                            const auto coin = _coins.require_find(id, "The coin not found.");
                            if (coin->type == toType(xx, yy)) {
                                frozencoins_t frozencoins(_self, _self.value);
                                auto coin = frozencoins.find(id);
                                if (coin == frozencoins.end())
                                    frozencoins.emplace(_self, [&](auto &c) { c.id = id; });
                            }
                        }
                    }
                }
            }
        }

        [[eosio::action]] void frozencoin(const uint64_t &id) {
            require_auth(_self);
            frozencoins_t frozencoins(_self, _self.value);
            auto coin = frozencoins.find(id);
            if (coin == frozencoins.end())
                frozencoins.emplace(_self, [&](auto &c) { c.id = id; });
        }

        // Dev
        // Test
        ACTION test() {
            //require_auth(_self);
            require_auth("megumimegumi"_n);
            /* ===== */
            auto owner = "heiheihahaha"_n;

            uint32_t type = 29;
            /* ===== */
            
            eosio_assert( type <= 22 + 6 + 1, "Type error");
            type --;

            collection_t coll(_self, owner.value);
            auto itr = coll.get_or_create(_self, st_collection { .records = vector<uint64_t> (22 + 6 + 1,0) } );

            uint64_t r ;
            if ( type <= 27 ) {
                const auto &v = collection_combination_parameters(type);
                for(auto&& yy : v) {
                    collection_counter(owner, yy, r);
                }
            } else if ( type == 28 ) {
                for (uint32_t i = 0 ; i < 22 ; ++i) {
                    if ( i == 0 ) r = itr.records[i];
                    if ( r > itr.records[i] ) r = itr.records[i];
                }
            }

            eosio_assert(r > itr.records[type], "Not Enough Coin");
            if ( type != 28 )
                token_mining_with_stake(owner, config::bouns_table(type), string{"Bouns from collection claim."});
            else
                _contract_dividend.collection_claim(owner); 
            
            SEND_INLINE_ACTION( *this, reccollclaim, { _self, "active"_n }, { owner, type } );
            itr.records[type] ++;
            coll.set(itr, _self);
            
            /* ===== */
            eosio_assert(false, "testX");
        }

        ACTION recharge(name from, asset amount, string memo) {
            require_auth(from);
            action(permission_level{_self, "active"_n},
               "ceshiyongeos"_n, "recharge"_n,
               make_tuple(from, amount, memo))
            .send();
        }

    public:
        void apply(uint64_t receiver, uint64_t code, uint64_t action) ;
};

vector<uint32_t> cryptojinian::merge_seed(const capi_checksum256 &s) {
    uint32_t hash ;
    // uint16_t s16[4] ;
    vector<uint32_t> v_hash ;
    for (uint32_t i = 0 ; i < 32 ; i += 4 ) {
        hash = ( s.hash[i] << 24 ) | ( s.hash[i+1] << 16 ) | ( s.hash[i+2] << 8 ) | s.hash[i+3] ;
        v_hash.push_back( hash ) ;
        hash = ( s.hash[i+3] << 24 ) | ( s.hash[i+2] << 16 ) | ( s.hash[i+1] << 8 ) | s.hash[i] ;
        v_hash.push_back( hash ) ;
        // hash = ( s16[0] << 48 ) | ( s16[1] << 32 ) | ( s16[2] << 16 )  | s16[3] ;
    }
    return v_hash;
}

void cryptojinian::init() {
    require_auth(_self);
    _global.set( st_global{ .id = 0, .remainamount = 429600 } , get_self() );
    _contract_kyubey.create(_self, asset(CCC_MAX_SUPPLY, CCC_SYMBOL) );
}

void cryptojinian::clear() {
    require_auth(_self);
    // auto itr = _players.begin();
    // while (itr != _players.end()) {
    //     _players.erase(itr);
    //     itr = _players.begin();
    // }
    // auto itr3 = _coins.begin();
    // while (itr3 != _coins.end()) {
    //     _coins.erase(itr3);
    //     itr3 = _coins.begin();
    // }
    // auto it = db_lowerbound_i64(get_self().value, get_self().value, ("dividend"_n).value , 0);
    // while (it >= 0) {
    //     auto del = it;
    //     uint64_t dummy;
    //     it = db_next_i64(it, &dummy);
    //     db_remove_i64(del);
    // }

    // auto it = db_lowerbound_i64(get_self().value, get_self().value, ("global"_n).value , 0);
    // while (it >= 0) {
    //     auto del = it;
    //     uint64_t dummy;
    //     it = db_next_i64(it, &dummy);
    //     db_remove_i64(del);
    // }

}

void cryptojinian::apply(uint64_t receiver, uint64_t code, uint64_t action) {
    auto &thiscontract = *this;
    if ( code == ( "eosio.token"_n ).value && action == ( "transfer"_n ).value ) {
        auto transfer_data = unpack_action_data<st_transfer>();
        onTransfer(transfer_data.from, transfer_data.to, transfer_data.quantity, transfer_data.memo);
        return;
    }

    if (code != get_self().value) return;
    switch (action) {
        EOSIO_DISPATCH_HELPER(cryptojinian,
                  (init)
                  (clear)
                  (issue)
                  (burn)
                  (transfer)
                  (ownersetcoin)
                  (ownerdelcoin)
                  (exchange)
                  (exchangedown)
                  (unfreezecoin)
                  (mining)
                  (pushorder)
                  (cancelorder)
                  (syscxlorder)
                  /*(takeorder)*/
                  (claim)
                  (collclaim)
                  (joinbuybackq)
                  (outbuybackq)
                  (cleanbbq)
                  (buyback)
                  (autobuyback)
                  (receipt)
                  (recmining)
                  (reccollclaim)
                  (recpcoll)
                  (frozen)
                  (frozencoin)
                  (test)
                  (recharge)
        )
    }
}
        
extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        cryptojinian p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(receiver, code, action);
        eosio_exit(0);
    }
}
