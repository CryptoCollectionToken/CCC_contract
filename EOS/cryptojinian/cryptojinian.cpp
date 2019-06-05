#include "cryptojinian.hpp"

bool cryptojinian::cd_check( const uint64_t &id )
{
   frozencoins_t frozencoins(_self, _self.value);
   auto itr = frozencoins.find(id);
   return itr == frozencoins.end(); // now() > itr->time_limit;
}
/*
bool cryptojinian::cd_check( const name &owner, const uint32_t &type )
{
   singleton_collcd_t collcd(_self, owner.value);
   auto itr = collcd.get_or_create(_self, st_collection_cd { .time_limit = vector<uint32_t> (22 + 6 + 1, now()-1) } );
   return now() > itr.time_limit[type];
}*/

void cryptojinian::update_frozen_time_limit( const name &owner, const uint32_t &type, const uint64_t &quantity, const uint32_t &frozen_days )
{
    frozencoins_t frozencoins(_self, _self.value);
    auto itr_p = _players.require_find(owner.value, "Player not found.") ;
    const auto &v_param = collection_combination_parameters(type);
    for (const auto &id : itr_p->coins) {
        if (cd_check(id)) {
            for (auto i = 0; i < quantity; ++i) {
                for(const auto& yy : v_param) {
                    for ( uint8_t xx = 0 ; xx < _coinvalues[yy].size(); ++xx ) {
                        const auto itr_coin = _coins.require_find(id, "The coin not found.");
                        if (itr_coin->type == toType(xx, yy)) {
                            auto n = frozencoins.find(id);
                            if ( n == frozencoins.end() )
                                frozencoins.emplace(_self, [&](auto &c) { c.id = id; });
                        }
                    }
                }
            }
        }
    }
}
/*
void cryptojinian::update_frozen_time_limit( const name &owner, const uint32_t &type, const uint32_t &frozen_days )
{
   singleton_collcd_t collcd(_self, owner.value);
   auto itr = collcd.get_or_create(_self, st_collection_cd { .time_limit = vector<uint32_t> (22 + 6 + 1, now()-1) } );
   itr.time_limit[type] = now() + (frozen_days * 86400);
   collcd.set(itr,_self);
}*/

void cryptojinian::setcoin(const name &owner, const uint64_t &type, const uint64_t &number) {
    //two-way binding.
    auto itr = _players.require_find( owner.value, "Unable to find player" );
    auto itr_newcoin = _coins.emplace(get_self(), [&](auto &c) {
        c.id = _coins.available_primary_key();
        c.owner = owner.value;
        c.type = type;
        c.number = number;
    });

    _players.modify(itr, get_self(), [&](auto &p) {
        p.coins.push_back(itr_newcoin->id);
    });
}

void cryptojinian::unfreezecoin(const uint64_t &id){
    require_auth(_self);
    frozencoins_t frozencoins(_self, _self.value);    
    auto itr = frozencoins.find(id);
    if (itr != frozencoins.end()) frozencoins.erase(itr);

    auto coin = _coins.require_find(id, "Unable to find coin.");
    frozencoins_t frozencoinsx(_self, coin->owner);
    auto itr2 = frozencoinsx.find(id);
    if (itr2 == frozencoinsx.end())
        frozencoinsx.emplace(_self, [&](auto &c) { c.id = id; });
}

void cryptojinian::deletecoin(const uint64_t &id) {
    auto onecoin = _coins.find(id);
    auto itr_player = _players.find(onecoin->owner);

    // 修改 coins 表
    _coins.modify(onecoin, get_self(), [&](auto &onecoin) {
        onecoin.owner = get_self().value;
    });

    // 修改凍結表
    frozencoins_t frozencoins(_self, _self.value);
    auto c_itr = frozencoins.find(id);
    if ( c_itr != frozencoins.end() ) // 有找到
        frozencoins.erase(c_itr) ;

    // 修改 players 表
    for(std::size_t i3=0;i3<itr_player->coins.size();i3++){
        if(itr_player->coins[i3] == id){
            _players.modify(itr_player, get_self(), [&](auto &p) {
                p.coins.erase(p.coins.begin()+i3);
            });

            break;
        }
    }
}

void cryptojinian::exchangecoin(const name &owner, const uint64_t &id) {
    // require_auth(get_self());
    //two-way binding.
    auto onecoin = _coins.find(id);
    auto itr = _players.require_find(owner.value, "Unable to find player");

    _coins.modify(onecoin, _self, [&](auto &onecoin) {
        onecoin.owner = owner.value;
    });

    _players.modify(itr, get_self(), [&](auto &p) {
        p.coins.push_back(id);
    });
}

uint64_t cryptojinian::addcoincount( uint64_t type ){
    auto usedcoins = _usedcoins.find( type );
    uint64_t globalcoincount = 0 ;
    globalcoincount ++;
    if (usedcoins == _usedcoins.end()) {
        _usedcoins.emplace(get_self(), [&](auto &u) {
            u.key = type;
            u.value = globalcoincount;
        });
    } else {
        _usedcoins.modify(usedcoins, get_self(), [&](auto &u) {
            u.value = globalcoincount += usedcoins->value ;
        });
    }
    return globalcoincount;
}

uint64_t cryptojinian::findcoinpos(uint32_t &input){
    const uint64_t pos = input % 429600;
    return pos;
}

void cryptojinian::newcoinbypos(const name owner, const uint64_t pos){
    // pos: 1~640000
    eosio_assert(pos <= 640000, "Pos Too Large.");  
    const uint64_t type_array[22] = {2100,10000,8400,19000,50000,14000,27000,10000,30000,10000,1620,10000,28000,10000,20000,27000,21000,37400,31100,23180,20000,19800};
    uint64_t pos_number = pos;
    uint64_t pos_count = 0;
    uint64_t array_count = 0;
    uint64_t type = 0;
    for(int i = 0; i < 22; i++){
        pos_count += type_array[i];
        array_count ++;
        if (pos <= pos_count){
            type = array_count;
            break;
        }
        pos_number -= type_array[i];
    }
    auto g = _global.get() ;
    g.remainamount -= 1;
    _global.set( g, _self) ;
    uint64_t globalcoincount = addcoincount(type);
    setcoin(owner,type,globalcoincount);
}

void cryptojinian::exchange(const std::string inputstrs){
    // input ids.
    std::vector<uint64_t> inputs;
    SplitString(inputstrs, inputs, ",");
    uint64_t coincount = inputs.size();
    uint64_t type = 0;
    name coinowner;
    auto onecoin = _coins.begin();
    for(int i=0;i<inputs.size();i++){
        eosio_assert(cd_check(inputs[i]), "This coin cant exchange, it is frozen.");
        onecoin = _coins.find(inputs[i]);
        require_auth(name(onecoin->owner));
        if (type == 0) {
            type = onecoin->type;
            coinowner = name(onecoin->owner);
        } else eosio_assert(type == onecoin->type, "Not Equal Type");  
    }

    uint64_t inputtype = type % 100;
    uint64_t inputvalue = type / 100;
    uint64_t coinvalue = _coinvalues[inputtype-1][inputvalue];
    uint64_t newcointype = 0;
    for(int i1=0 ; i1 < _coinvalues[inputtype-1].size() ; i1++){
        if ((coincount * coinvalue == _coinvalues[inputtype-1][i1]) && (i1 > inputvalue)){
            for(int i2=0;i2<inputs.size();i2++){
                deletecoin(inputs[i2]);
            }
            newcointype = (i1 * 100) + inputtype;
            uint64_t globalcoincount = addcoincount(newcointype);
            setcoin(coinowner,newcointype,globalcoincount);
        }
    }
}

void cryptojinian::exchangedown(const uint64_t inputid, const uint64_t goal){
    auto onecoin = _coins.begin();
    eosio_assert(cd_check(inputid), "This coin cant exchange, it is frozen.");
    onecoin = _coins.find(inputid);
    require_auth(name(onecoin->owner));
    uint64_t goaltype = goal % 100;
    uint64_t goalvalue = goal / 100;
    uint64_t inputtype = onecoin->type%100;
    uint64_t inputvalue = onecoin->type/100;
    eosio_assert(inputtype == goaltype, "Not Equal Type");  
    eosio_assert(goalvalue < inputvalue, "Goal Is Gearter Than Input");
    uint64_t amount = _coinvalues[inputtype-1][inputvalue]/_coinvalues[goaltype-1][goalvalue];
    eosio_assert(_coinvalues[inputtype-1][inputvalue]%_coinvalues[goaltype-1][goalvalue] == 0, "Cant't exactly divided.");
    for(int i1 = 0; i1 < amount; i1++){
        if(goalvalue == 0){
            for(int i2 = 0; i2 < _coins.available_primary_key(); i2++){
                auto onecoin_finder = _coins.find(i2);
                if(onecoin_finder == _coins.end()) continue;
                if(onecoin_finder->owner != get_self().value) continue;
                if(onecoin_finder->type != goal) continue;
                exchangecoin(name(onecoin->owner),i2);
                break;
            }
        }else{
            uint64_t globalcoincount = addcoincount(goal);
            setcoin(name(onecoin->owner),goal,globalcoincount);
        }
    }
    deletecoin(inputid);
}

void cryptojinian::token_mining(name miner, asset quantity, string memo)
{
    // require_auth(get_self());
    _contract_kyubey.no_permission_issue(miner, quantity, memo);
    // SEND_INLINE_ACTION failed !
    /*
    SEND_INLINE_ACTION( _contract_kyubey, issue, {get_self(),"active"_n},
                        {itr->miner, asset( string_to_price("1.0000"), CCC_SYMBOL ), "mining 1 CCC"} );*/
}

void cryptojinian::token_mining_with_stake(name miner, asset quantity, string memo)
{
    token_mining( miner, quantity, memo );
    _contract_dividend.stake( miner, quantity );
}

// check cost 
// join_game_processing(miner)
// join mining waiting Q
void cryptojinian::join_miningqueue(const name &miner, const asset &totalcost)
{
    // cost check
    const auto &mc = _global.get().miningcost();
    const int64_t times = totalcost / mc; 
    eosio_assert(times > 0, "You have wrong cost." );
    eosio_assert(times <= 10, "You have mining too much times.");
    // eosio_assert(false, int_to_string( times ).c_str() );

    join_game_processing(miner);

    // join mining waiting Q
    miningqueue_t _miningqueue(get_self(), get_self().value);
    for (uint8_t n = 0; n < times; n++) {
        token_mining_with_stake(miner, asset{mc.amount * config::MINING_COEF, TOKEN_SYMBOL}, string{"CCC mining."});
        _miningqueue.emplace( _self, [&](auto &q) {
            q.id = _miningqueue.available_primary_key();
            q.miner = miner.value;
        });
    }
}

void cryptojinian::ref_processing( const name &miner, const name &sponsor )
{   
    auto itr_sponsor = join_game_processing( sponsor ) ;
    auto itr_miner = _players.find(miner.value);
    
    if (name(itr_miner->sponsor) != DEF_SPONSOR)
        return;
    else if (std::find(itr_sponsor->refs.begin(), itr_sponsor->refs.end(), miner.value) == itr_sponsor->refs.end())
    {
        _players.modify(itr_sponsor, get_self(), [&](auto &s) {
            s.refs.push_back(miner.value);
        });
        // 發 bouns token 給 sponsor
        const auto refs_size = itr_sponsor->refs.size();
        asset bouns( 0, config::TOKEN_SYMBOL );
        if (refs_size <= 10)
            bouns.set_amount( 10 * 10000 );
        else if (refs_size <= 30)
            bouns.set_amount( 12 * 10000 );
        else if (refs_size <= 70)
            bouns.set_amount( 15 * 10000 );
        else if (refs_size <= 100)
            bouns.set_amount( 20 * 10000 );
        else
            bouns.set_amount( 25 * 10000 );

        token_mining_with_stake( name(itr_sponsor->playername), bouns,
                      "bouns " + std::to_string(bouns.amount / 10000) + " CCC" );
            
        _players.modify(itr_miner, get_self(), [&](auto &m) {
            m.sponsor = sponsor.value;
        });
    } // else if
} // ref_processing()

inline const asset cryptojinian::fee_processing(asset &quantity) {
    auto delta = quantity.amount * TRADE_COEF;
    if (delta >= 1) {
        _contract_dividend.make_profit( quantity.amount - delta, _contract_kyubey.get_supply(TOKEN_SYMBOL));
        quantity.set_amount(delta);
    }
    return quantity;
}

// check trade id & paid EOS
// transfer coin 所有权
// issue CCC bouns to buyer
// 调整价格回原价
// transfer EOS to seller
// make log
// del order
void cryptojinian::takeorder(const name &buyer, const uint64_t &order_id, asset &eos ) {
    order_t _orders( _self, _self.value );

    // check trade id & paid EOS
    auto itr_order = _orders.require_find(order_id, "Trade id is not found" );
    eosio_assert(itr_order->bid == eos, "Asset does not match");
    
    const auto seller = name(itr_order->account);

    // transfer coin 所有权
    auto itr_player = _players.find( seller.value );
    auto &v_pcoins = itr_player->coins;
    for (const auto &cid : itr_order->the_coins_for_sell){
        // deletecoin(cid);
        for(std::size_t i=0;i<v_pcoins.size();++i){
            if( cid ==  v_pcoins[i] ){
                _players.modify(itr_player, get_self(), [&](auto &p) {
                    p.coins.erase(p.coins.begin()+i);
                });
                auto onecoin = _coins.find(cid);
                uint64_t inputtype = onecoin->type % 100;
                uint64_t inputvalue = onecoin->type / 100;
                uint64_t amount = _coinvalues[inputtype-1][inputvalue] / _coinvalues[inputtype-1][0];
                token_unstake_and_burn(seller, asset( amount * 32 * 10000, config::TOKEN_SYMBOL ), string{""});
                break;
            }
        }
        exchangecoin(buyer, cid);
    }
    
    // issue token bouns to buyer
    token_mining_with_stake( buyer, asset( eos.amount, config::TOKEN_SYMBOL ), "CCC bouns" );

    // 调整价格回原价
    eos.set_amount(eos.amount / ( 1 + TRADE_FEE ));

    // transfer EOS to seller
    auto delta = fee_processing( eos ) ;
    if ( delta.amount > 0){
        action(permission_level{_self, "active"_n},
               "eosio.token"_n, "transfer"_n,
               make_tuple(_self, seller, delta, std::string{""}))
            .send();
        /* string("Trade ") + to_string(order_id) + string(" be took") */
    }

    // make log
    const st_rec_takeOrder _tor{
        .matched_order = *itr_order,
        .buyer = buyer,
    };

    action(permission_level{_self, "active"_n},
           get_self(), "receipt"_n, _tor)
        .send();

    // del order
    _orders.erase(itr_order);
} // take_order()

void cryptojinian::SplitString(const std::string& s, vector<uint64_t>& v, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        // strtoull(pszValue, NULL, 0)
        v.push_back(std::strtoull(s.substr(pos1, pos2-pos1).c_str(), NULL, 0));
         
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(std::strtoull(s.substr(pos1, pos2-pos1).c_str(), NULL, 0));
}

void cryptojinian::onTransfer(name from, name to, asset quantity, std::string memo) {            
    if (from == _self || to != _self) return;   
    require_auth(from);
    eosio_assert(quantity.is_valid(), "invalid token transfer");
    eosio_assert(quantity.symbol == EOS_SYMBOL, "only EOS token is allowed");
    eosio_assert(quantity.amount > 0, "Must transfer a positive amount"); // 正數的結界

    auto params = explode(memo, ' ');
    eosio_assert(params.size() <= 5, "Error memo");

    if (params[0] == "mining") { // mining
        join_miningqueue(from, quantity);
        if ( params.size() < 3 )
            ref_processing( from ) ;
        else { // mining ref <sponsor>
            eosio_assert(params.size() == 3 && params[1] == "ref", "Error memo");

            name sponsor( params[2].c_str() ) ;
            eosio_assert( is_account(sponsor), "Sponsor is not an existing account."); // sponsor 存在 check
            ref_processing( from, sponsor );
        }
        
        /*
            Older code mechanism:
                stake => 100CCC stake for someone
                make_profit => 50EOS / total_staked == get_supply // ex: 200 CCC
                token_mining => +100CCC to someone
                
                Wrong, total_staked should be 300 CCC

            Newer code mechanism:
                token_mining_with_stake => +100CCC & stake for someone
                make_profit => 50EOS / total_staked == get_supply // would be 300 CCC
            
            Never beaker couple-thing which should be always done together.
        */
        _contract_dividend.make_profit( quantity.amount, _contract_kyubey.get_supply( TOKEN_SYMBOL ) );
        return;
    }

    if (params[0] == "take_order") {
        eosio_assert(params.size() == 2, "Error memo");
        uint64_t order_id = string_to_int(params[1]) ;

        takeorder( from, order_id, quantity );
        //SEND_INLINE_ACTION( *this, takeorder, { from, "active"_n }, { from, order_id, quantity } );
        return;
    }
}