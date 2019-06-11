#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>

#include <string>

namespace kyubeytool {

   using std::string;
   using namespace config ;

   class dividend : public contract {
      public:
         dividend( name receiver, name code, datastream<const char*> ds ) :
            contract( receiver, code, ds ),
            _global( receiver, receiver.value ){}

         static constexpr uint128_t MAGNITUDE = 1ll<<32;

         struct st_player_info {
            // asset staked;
            asset payout = asset(0, config::DIVIDEND_EX_SYMBOL.get_symbol());
         };
         /*
         struct refund_request {
            time request_time;
            asset amount;
         };*/
         void cleanbuyback();

         void make_profit(uint64_t delta, asset total_staked);
         void stake(name &from, asset delta);
         void unstake(name from, asset quantity, string memo);
         void claim(name &from, asset quantity);
         void collection_claim( const name &from );
         void refund(name from);

         struct st_d_global {
            uint64_t defer_id = 0 ;
            uint128_t earnings_per_share = 0 ;
            uint64_t earnings_for_buyback = 0 ;
            uint64_t earnings_for_collection = 0 ;
            capi_name last;

            auto primary_key() const { return defer_id; }
            EOSLIB_SERIALIZE(st_d_global, (defer_id)(earnings_per_share)(earnings_for_buyback)(earnings_for_collection)(last)) 
         };

         typedef singleton<"dividend"_n, st_d_global> singleton_global_t;
         typedef singleton<"playerinfo"_n, st_player_info> singleton_playerinfo_t;
         singleton_global_t _global;

         uint64_t get_next_defer_id() {
            auto g = _global.get();    
            g.defer_id ++;
            _global.set(g, get_self());
            return g.defer_id;
         }

         template <typename... Args>
         void send_defer_action(Args&&... args) {
            transaction trx;
            trx.actions.emplace_back(std::forward<Args>(args)...);
            trx.send(get_next_defer_id(), get_self(), false);
         }

   };

void dividend::cleanbuyback() {
    auto g = _global.get_or_create(_self, st_d_global{});
    g.earnings_for_buyback = 0 ; 
    _global.set(g, _self);
}

void dividend::make_profit(uint64_t delta, asset total_staked) {
    auto g = _global.get_or_create(_self, st_d_global{});
    g.earnings_per_share += MAGNITUDE * delta * DIVIDEND_COEF / total_staked.amount;
    g.earnings_for_buyback += delta * BUYBACK_COEF ; 
    g.earnings_for_collection += delta * COLLECTION_COEF ;
    _global.set(g, _self);
}

void dividend::stake(name &from, asset delta) {
   eosio_assert(delta.amount > 0, "must stake a positive amount");
   eosio_assert(delta.symbol == config::TOKEN_SYMBOL, "must be token.");

   singleton_playerinfo_t _playerinfo(_self, from.value);
   auto pi = _playerinfo.get_or_create(_self, st_player_info{});
   auto g = _global.get_or_create(_self, st_d_global{});

   int64_t g_payout = g.earnings_per_share * delta.amount / MAGNITUDE ;
   /*
   if ( g_payout < string_to_price("0.0001") )
      g_payout = string_to_price("0.0001");
   */
   // only " asset += asset " has addition underflow & overflow checking
   pi.payout += asset(g_payout, pi.payout.symbol);
   _playerinfo.set(pi, _self);
}
/*
 * 假設所有的CCC issue, burn 時都有正確被 stake & unstke
 * 那麼只有出錯的時候，payout 才會是 0
*/
void dividend::claim(name &owner, asset quantity) {
   require_auth(_self);

   singleton_playerinfo_t _playerinfo(_self, owner.value);
   auto pi = _playerinfo.get_or_create(_self, st_player_info{});
   auto g = _global.get();

   auto delta = extended_asset(0, config::DIVIDEND_EX_SYMBOL);
   int64_t raw_dividend = g.earnings_per_share * quantity.amount / MAGNITUDE;
   if (raw_dividend > pi.payout.amount)
       delta.quantity.set_amount(raw_dividend - pi.payout.amount);

   if ( quantity.amount > 0 && pi.payout.amount == 0 ) {
      pi.payout.set_amount(raw_dividend);
      _playerinfo.set(pi, _self);
      return ;
   }

   pi.payout.set_amount(raw_dividend);
   _playerinfo.set(pi, _self);

   eosio_assert(delta.quantity.is_valid() && delta.quantity.amount > 0, "Can't Send EOS.");
   action(permission_level{_self, "active"_n},
          delta.get_extended_symbol().get_contract(), "transfer"_n,
          make_tuple(_self, owner, delta.quantity, string("claim dividend.")))
   .send();

   g.last = owner.value;
   _global.set(g, _self);
}

void dividend::collection_claim(const name &from) {
   // require_auth(_self);
    auto g = _global.get();
    int64_t raw_dividend = g.earnings_for_collection;
    asset delta( raw_dividend, config::EOS_SYMBOL);

    if ( delta.is_valid() && delta.amount > 0) { 
      action(permission_level{ _self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            make_tuple( _self, from, delta,
                string("Claim collection bouns."))
      ).send();

      g.earnings_for_collection = 0 ;
      _global.set(g, _self);       
    } else {
      eosio_assert(false, "Can't Send EOS.");
   }
}

void dividend::unstake(name from, asset quantity, string memo)
{
   eosio_assert(quantity.amount > 0, "must stake a positive amount");
   eosio_assert(quantity.symbol == config::TOKEN_SYMBOL, "must be token.");

   singleton_playerinfo_t _playerinfo(_self, from.value);
   // eosio_assert(_playerinfo.exists(), "stake info not found" );
   // eosio_assert(quantity <= v.staked, "don't have enough token for unstake");
   // auto pi = _playerinfo.get();
   auto pi = _playerinfo.get_or_create(_self, st_player_info{});
   auto g = _global.get();

   int64_t g_payout = g.earnings_per_share * quantity.amount / MAGNITUDE ;
   if ( asset{g_payout, pi.payout.symbol} <= pi.payout) {
     // only " asset -= asset " has addition underflow & overflow checking
     pi.payout -= asset(g_payout, pi.payout.symbol);
   } else {
     pi.payout.set_amount(0);
   }
   _playerinfo.set(pi, _self);
}

void dividend::refund(name from)
{
    require_auth( from );
    /*
    singleton_refunds refunds_tbl( _self, from.value );
    eosio_assert( refunds_tbl.exists(), "refund request not found" );
    auto req = refunds_tbl.get();

    // eosio_assert( req.request_time + refund_delay >= now(), "refund is not available yet" );
    // Until now() becomes NOW, the fact that now() is the timestamp of the previous block could in theory
    // allow people to get their tokens earlier than the 1 day delay if the unstake happened immediately after many
    // consecutive missed blocks.

    action(
        permission_level{_self, "active"_n},
        TOKEN_CONTRACT, "transfer"_n,
        make_tuple(_self, from, req.amount, string("unstake refund"))
    ).send();

    refunds_tbl.remove();
    */
}

} // namespace kyubeytool