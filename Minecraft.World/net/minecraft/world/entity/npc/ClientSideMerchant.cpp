#include "../../../../../Header Files/stdafx.h"
#include "../../item/trading/net.minecraft.world.item.trading.h"
#include "../../inventory/net.minecraft.world.inventory.h"
#include "ClientSideMerchant.h"

ClientSideMerchant::ClientSideMerchant(std::shared_ptr<Player> source,
                                       const std::wstring& name) {
    this->source = source;
    // 4J Stu - Need to do this after creating as a shared_ptr
    container = nullptr;  // new MerchantContainer(source, this);
    currentOffers = nullptr;
    m_name = name;
}

ClientSideMerchant::~ClientSideMerchant() {
    delete container;
    delete currentOffers;
}

void ClientSideMerchant::createContainer() {
    container = new MerchantContainer(source, shared_from_this());
}

Container* ClientSideMerchant::getContainer() { return container; }

std::shared_ptr<Player> ClientSideMerchant::getTradingPlayer() {
    return source;
}

void ClientSideMerchant::setTradingPlayer(std::shared_ptr<Player> player) {}

MerchantRecipeList* ClientSideMerchant::getOffers(
    std::shared_ptr<Player> forPlayer) {
    return currentOffers;
}

void ClientSideMerchant::overrideOffers(MerchantRecipeList* recipeList) {
    delete currentOffers;
    currentOffers = recipeList;
}

void ClientSideMerchant::notifyTrade(MerchantRecipe* activeRecipe) {
    activeRecipe->increaseUses();
}

void ClientSideMerchant::notifyTradeUpdated(
    std::shared_ptr<ItemInstance> item) {}

std::wstring ClientSideMerchant::getDisplayName() { return m_name; }