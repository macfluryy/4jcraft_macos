#pragma once

#include "Merchant.h"

class MerchantContainer;
class MerchantRecipeList;
class MerchantRecipe;

class ClientSideMerchant : public Merchant, public std::enable_shared_from_this<ClientSideMerchant>
{
private:
	MerchantContainer *container;
    std::shared_ptr<Player> source;
    MerchantRecipeList *currentOffers;
	int m_name;

public:
	ClientSideMerchant(std::shared_ptr<Player> source, int name);
	~ClientSideMerchant();

	void createContainer(); // 4J Added
    Container *getContainer();
    std::shared_ptr<Player> getTradingPlayer();
    void setTradingPlayer(std::shared_ptr<Player> player);
    MerchantRecipeList *getOffers(std::shared_ptr<Player> forPlayer);
    void overrideOffers(MerchantRecipeList *recipeList);
    void notifyTrade(MerchantRecipe *activeRecipe);
	void notifyTradeUpdated(std::shared_ptr<ItemInstance> item);
	int getDisplayName();
};