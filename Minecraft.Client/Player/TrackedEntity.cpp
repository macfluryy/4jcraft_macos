#include "../Build/stdafx.h"
#include "TrackedEntity.h"
#include "ServerPlayer.h"
#include "../Network/PlayerConnection.h"
#include "../../Minecraft.World/Util/Mth.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.item.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.monster.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.player.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.animal.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.global.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.projectile.h"
#include "../../Minecraft.World/Headers/net.minecraft.network.packet.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.item.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.saveddata.h"
#include "../MinecraftServer.h"
#include "../Level/ServerLevel.h"
#include "../Network/PlayerList.h"
#include "EntityTracker.h"
#include "../Network/PlayerChunkMap.h"
#ifndef __linux__
#include <qnet.h>
#endif // __linux__

TrackedEntity::TrackedEntity(std::shared_ptr<Entity> e, int range, int updateInterval, bool trackDelta)
{
	// 4J added initialisers
    xap = yap = zap = 0;
    tickCount = 0;
	xpu = ypu = zpu = 0;
    updatedPlayerVisibility = false;
    teleportDelay = 0;
	moved = false;
	
	this->e = e;
    this->range = range;
    this->updateInterval = updateInterval;
    this->trackDelta = trackDelta;

    xp = Mth::floor(e->x * 32);
    yp = Mth::floor(e->y * 32);
    zp = Mth::floor(e->z * 32);

    yRotp = Mth::floor(e->yRot * 256 / 360);
    xRotp = Mth::floor(e->xRot * 256 / 360);
	yHeadRotp = Mth::floor(e->getYHeadRot() * 256 / 360);
}

int c0a = 0, c0b = 0, c1a = 0, c1b = 0, c1c = 0, c2a = 0, c2b = 0;

void TrackedEntity::tick(EntityTracker *tracker, vector<std::shared_ptr<Player> > *players)
{
    moved = false;
    if (!updatedPlayerVisibility || e->distanceToSqr(xpu, ypu, zpu) > 4 * 4)
	{
        xpu = e->x;
        ypu = e->y;
        zpu = e->z;
        updatedPlayerVisibility = true;
        moved = true;
        updatePlayers(tracker, players);
    }

	if (wasRiding != e->riding)
	{
		wasRiding = e->riding;
		broadcast(std::shared_ptr<SetRidingPacket>(new SetRidingPacket(e, e->riding)));
	}

	// Moving forward  special case for item frames
	if (e->GetType()== eTYPE_ITEM_FRAME && tickCount % 10 == 0) 
	{
		std::shared_ptr<ItemFrame> frame = dynamic_pointer_cast<ItemFrame> (e);
		std::shared_ptr<ItemInstance> item = frame->getItem();

		if (item != NULL && item->getItem()->id == Item::map_Id && !e->removed) 
		{
 			std::shared_ptr<MapItemSavedData> data = Item::map->getSavedData(item, e->level);
 			for (AUTO_VAR(it,players->begin() ); it != players->end(); ++it)
			{
 				std::shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(*it);
 				data->tickCarriedBy(player, item);
 
 				if (!player->removed && player->connection && player->connection->countDelayedPackets() <= 5) 
 				{
 					std::shared_ptr<Packet> packet = Item::map->getUpdatePacket(item, e->level, player);
 					if (packet != NULL) player->connection->send(packet);
 				}
 			}
		}

		std::shared_ptr<SynchedEntityData> entityData = e->getEntityData();
		if (entityData->isDirty()) 
		{
			broadcastAndSend( std::shared_ptr<SetEntityDataPacket>( new SetEntityDataPacket(e->entityId, entityData, false) ) );
		}
	}
	else
	{
		if(e->riding == NULL)
		{
			teleportDelay++;
			if (tickCount++ % updateInterval == 0 || e->hasImpulse)
			{
				int xn = Mth::floor(this->e->x * 32.0);
				int yn = Mth::floor(this->e->y * 32.0);
				int zn = Mth::floor(this->e->z * 32.0);
				int yRotn = Mth::floor(e->yRot * 256 / 360);
				int xRotn = Mth::floor(e->xRot * 256 / 360);


				int xa = xn - xp;
				int ya = yn - yp;
				int za = zn - zp;

				std::shared_ptr<Packet> packet = nullptr;

				// 4J - this pos flag used to be set based on abs(xn) etc. but that just seems wrong
				bool pos = abs(xa) >= TOLERANCE_LEVEL || abs(ya) >= TOLERANCE_LEVEL || abs(za) >= TOLERANCE_LEVEL;
				// 4J - changed rotation to be generally sent as a delta as well as position
				int yRota = yRotn - yRotp;
				int xRota = xRotn - xRotp;
				// Keep rotation deltas in +/- 180 degree range 
				while( yRota > 127 ) yRota -= 256;
				while( yRota < -128 ) yRota += 256;
				while( xRota > 127 ) xRota -= 256;
				while( xRota < -128 ) xRota += 256;

				bool rot = abs(yRota) >= TOLERANCE_LEVEL || abs(xRota) >= TOLERANCE_LEVEL;

				if (xa < -128 || xa >= 128 || ya < -128 || ya >= 128 || za < -128 || za >= 128
					// 4J Stu - I fixed the initialisation of teleportDelay in the ctor, but we managed this far without out
						// and would prefer not to have all the extra traffix so ignore it
							// 4J Stu - Fix for #9579 - GAMEPLAY: Boats with a player in them slowly sink under the water over time, and with no player in them they float into the sky.
								|| (e->GetType() == eTYPE_BOAT && teleportDelay > 20 * 20)
								)
				{
					teleportDelay = 0;
					packet = std::shared_ptr<TeleportEntityPacket>( new TeleportEntityPacket(e->entityId, xn, yn, zn, (uint8_t) yRotn, (uint8_t) xRotn) );
					//			printf("%d: New teleport rot %d\n",e->entityId,yRotn);
					yRotp = yRotn;
					xRotp = xRotn;
				}
				else
				{
					if (pos && rot)
					{
						// 4J If the movement is small enough, and there's no xrot, then use the new smaller packets
						if( ( xa >= -16 ) && ( xa <= 15 ) &&
							( za >= -16 ) && ( za <= 15 ) &&
							( ya >= -32 ) && ( ya <= 31 ) &&
							( xRota == 0 ))
						{
							// Clamp rotations that are too big
							if( yRota < -16 )
							{
								yRota = -16;
								yRotn = yRotp + yRota;
							}
							else if( yRota > 15 )
							{
								yRota = 15;
								yRotn = yRotp + yRota;
							}
							// 5 bits each for x & z, and 6 for y
							packet = std::shared_ptr<MoveEntityPacketSmall>( new MoveEntityPacketSmall::PosRot(e->entityId, (char) xa, (char) ya, (char) za, (char) yRota, 0 ) );
							c0a++;
						}
						else
						{
							packet = std::shared_ptr<MoveEntityPacket>( new MoveEntityPacket::PosRot(e->entityId, (char) xa, (char) ya, (char) za, (char) yRota, (char) xRota) );
							//					printf("%d: New posrot %d + %d = %d\n",e->entityId,yRotp,yRota,yRotn);
							c0b++;
						}
					}
					else if (pos)
					{
						// 4J If the movement is small enough, then use the new smaller packets
						if( ( xa >= -8 ) && ( xa <= 7 ) &&
							( za >= -8 ) && ( za <= 7 ) &&
							( ya >= -16 ) && ( ya <= 15 ) )
						{
							// 4 bits each for x & z, and 5 for y
							packet = std::shared_ptr<MoveEntityPacketSmall>( new MoveEntityPacketSmall::Pos(e->entityId, (char) xa, (char) ya, (char) za) );
							c1a++;
						}

						else if( ( xa >= -16 ) && ( xa <= 15 ) &&
							( za >= -16 ) && ( za <= 15 ) &&
							( ya >= -32 ) && ( ya <= 31 ) )
						{
							// use the packet with small packet with rotation if we can - 5 bits each for x & z, and 6 for y - still a byte less than the alternative
							packet = std::shared_ptr<MoveEntityPacketSmall>( new MoveEntityPacketSmall::PosRot(e->entityId, (char) xa, (char) ya, (char) za, 0, 0 ));
							c1b++;
						}
						else
						{
							packet = std::shared_ptr<MoveEntityPacket>( new MoveEntityPacket::Pos(e->entityId, (char) xa, (char) ya, (char) za) );
							c1c++;
						}
					}
					else if (rot)
					{
						// 4J If there's no x rotation, then use the new smaller packet type
						if( xRota == 0 )
						{
							// Clamp rotations that are too big
							if( yRota < -16 )
							{
								yRota = -16;
								yRotn = yRotp + yRota;
							}
							else if( yRota > 15 )
							{
								yRota = 15;
								yRotn = yRotp + yRota;
							}
							packet = std::shared_ptr<MoveEntityPacketSmall>( new MoveEntityPacketSmall::Rot(e->entityId, (char) yRota, 0) );
							c2a++;
						}
						else
						{
							//					printf("%d: New rot %d + %d = %d\n",e->entityId,yRotp,yRota,yRotn);
							packet = std::shared_ptr<MoveEntityPacket>( new MoveEntityPacket::Rot(e->entityId, (char) yRota, (char) xRota) );
							c2b++;
						}
					}
				}

				if (trackDelta)
				{
					double xad = e->xd - xap;
					double yad = e->yd - yap;
					double zad = e->zd - zap;

					double max = 0.02;

					double diff = xad * xad + yad * yad + zad * zad;

					if (diff > max * max || (diff > 0 && e->xd == 0 && e->yd == 0 && e->zd == 0))
					{
						xap = e->xd;
						yap = e->yd;
						zap = e->zd;
						broadcast( std::shared_ptr<SetEntityMotionPacket>( new SetEntityMotionPacket(e->entityId, xap, yap, zap) ) );
					}

				}

				if (packet != NULL)
				{
					broadcast(packet);
				}

				std::shared_ptr<SynchedEntityData> entityData = e->getEntityData();

				if (entityData->isDirty())
				{
					broadcastAndSend( std::shared_ptr<SetEntityDataPacket>( new SetEntityDataPacket(e->entityId, entityData, false) ) );
				}

				int yHeadRot = Mth::floor(e->getYHeadRot() * 256 / 360);
				if (abs(yHeadRot - yHeadRotp) >= TOLERANCE_LEVEL)
				{
					broadcast(std::shared_ptr<RotateHeadPacket>(new RotateHeadPacket(e->entityId, (uint8_t) yHeadRot)));
					yHeadRotp = yHeadRot;
				}

				if (pos)
				{
					xp = xn;
					yp = yn;
					zp = zn;
				}
				if (rot)
				{
					yRotp = yRotn;
					xRotp = xRotn;
				}

				//		if( dynamic_pointer_cast<ServerPlayer>(e) != NULL )
				//		{
				//			printf("%d: %d + %d = %d (%f)\n",e->entityId,xRotp,xRota,xRotn,e->xRot);
				//		}
			}
			
		}
		else // 4J-JEV: Added: Mobs in minecarts weren't synching their invisibility.
		{
			std::shared_ptr<SynchedEntityData> entityData = e->getEntityData();
			if (entityData->isDirty())
				broadcastAndSend( std::shared_ptr<SetEntityDataPacket>( new SetEntityDataPacket(e->entityId, entityData, false) ) );
		}
		e->hasImpulse = false;
	}

    if (e->hurtMarked)
	{
        // broadcast(new AnimatePacket(e, AnimatePacket.HURT));
        broadcastAndSend( std::shared_ptr<SetEntityMotionPacket>( new SetEntityMotionPacket(e) ) );
        e->hurtMarked = false;
    }

}

void TrackedEntity::broadcast(std::shared_ptr<Packet> packet)
{
	if( Packet::canSendToAnyClient( packet ) )
	{
		// 4J-PB - due to the knockback on a player being hit, we need to send to all players, but limit the network traffic here to players that have not already had it sent to their system
		vector< std::shared_ptr<ServerPlayer> > sentTo;

		// 4J - don't send to a player we've already sent this data to that shares the same machine. 
		// EntityMotionPacket used to limit themselves to sending once to each machine
		// by only sending to the primary player on each machine. This was causing trouble for split screen
		// as only the primary player would get a knockback velocity. Now these packets can be sent to any
		// player, but we try to restrict the network impact this has by not resending to the one machine

		for( AUTO_VAR(it, seenBy.begin()); it != seenBy.end(); it++ )
		{
			std::shared_ptr<ServerPlayer> player = *it;
			bool dontSend = false;
			if( sentTo.size() )
			{
				INetworkPlayer *thisPlayer =player->connection->getNetworkPlayer();
				if( thisPlayer == NULL )
				{
					dontSend = true;
				}
				else
				{
					for(unsigned int j = 0; j < sentTo.size(); j++ )	
					{
						std::shared_ptr<ServerPlayer> player2 = sentTo[j];
						INetworkPlayer *otherPlayer = player2->connection->getNetworkPlayer();
						if( otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer) )
						{
							dontSend = true;
		// #ifdef _DEBUG
		// 					std::shared_ptr<SetEntityMotionPacket> emp= dynamic_pointer_cast<SetEntityMotionPacket> (packet);
		// 					if(emp!=NULL)
		// 					{
		// 						app.DebugPrintf("Not sending this SetEntityMotionPacket to player - it's already been sent to a player on their console\n");
		// 					}
		// #endif
						}
					}
				}
			}
			if( dontSend )
			{
				continue;
			}


			(*it)->connection->send(packet);
			sentTo.push_back(player);
		}
	}
	else
	{
		// This packet hasn't got canSendToAnyClient set, so just send to everyone here, and it

		for( AUTO_VAR(it, seenBy.begin()); it != seenBy.end(); it++ )
		{
			(*it)->connection->send(packet);
		}
	}
}

void TrackedEntity::broadcastAndSend(std::shared_ptr<Packet> packet)
{
	vector< std::shared_ptr<ServerPlayer> > sentTo;
    broadcast(packet);
	std::shared_ptr<ServerPlayer> sp = dynamic_pointer_cast<ServerPlayer>(e);
	if (sp != NULL && sp->connection)
	{
        sp->connection->send(packet);
    }
}

void TrackedEntity::broadcastRemoved()
{
	for( AUTO_VAR(it, seenBy.begin()); it != seenBy.end(); it++ )
	{
		(*it)->entitiesToRemove.push_back(e->entityId);
	}
}

void TrackedEntity::removePlayer(std::shared_ptr<ServerPlayer> sp)
{
	AUTO_VAR(it, seenBy.find( sp ));
	if( it != seenBy.end() )
	{
		seenBy.erase( it );
	}
}

// 4J-JEV: Added for code reuse.
TrackedEntity::eVisibility TrackedEntity::isVisible(EntityTracker *tracker, std::shared_ptr<ServerPlayer> sp, bool forRider)
{
	// 4J Stu - We call update players when the entity has moved more than a certain amount at the start of it's tick
	// Before this call we set xpu, ypu and zpu to the entities new position, but xp,yp and zp are the old position until later in the tick.
	// Therefore we should use the new position for visibility checks
    double xd = sp->x - xpu; //xp / 32;
    double zd = sp->z - zpu; //zp / 32;

	int playersRange = range;
	if( playersRange > TRACKED_ENTITY_MINIMUM_VIEW_DISTANCE  )
	{
		playersRange -= sp->getPlayerViewDistanceModifier();
	}

	bool bVisible = xd >= -playersRange && xd <= playersRange && zd >= -playersRange && zd <= playersRange;
	bool canBeSeenBy = canBySeenBy(sp);

	// 4J - added. Try and find other players who are in the same dimension as this one and on the same machine, and extend our visibility
	// so things are consider visible to this player if they are near the other one. This is because we only send entity tracking info to
	// players who canReceiveAllPackets().
	if(!bVisible)
	{
		MinecraftServer *server = MinecraftServer::getInstance();
		INetworkPlayer *thisPlayer = sp->connection->getNetworkPlayer();
		if( thisPlayer )
		{
			for( unsigned int i = 0; i < server->getPlayers()->players.size(); i++ )
			{
				// Consider extra players, but not if they are the entity we are tracking, or the player we've been passed as input, or in another dimension
				std::shared_ptr<ServerPlayer> ep = server->getPlayers()->players[i];
				if( ep == sp ) continue;
				if( ep == e ) continue;
				if( ep->dimension != sp->dimension ) continue;

				INetworkPlayer * otherPlayer = ep->connection->getNetworkPlayer();
				if( otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer) )
				{
					// 4J Stu - We call update players when the entity has moved more than a certain amount at the start of it's tick
					// Before this call we set xpu, ypu and zpu to the entities new position, but xp,yp and zp are the old position until later in the tick.
					// Therefore we should use the new position for visibility checks
					double xd = ep->x - xpu; //xp / 32;
					double zd = ep->z - zpu; //zp / 32;
					bVisible |= ( xd >= -playersRange && xd <= playersRange && zd >= -playersRange && zd <= playersRange );
					canBeSeenBy |= canBySeenBy(ep);
				}
			}
		}
	}

	// 4J Stu - We need to ensure that we send the mount before the rider, so check that the player has been added to the seenBy list
	if(forRider)
	{
		canBeSeenBy = canBeSeenBy && (seenBy.find(sp) != seenBy.end());
	}

	// 4J-JEV: ADDED! An entities mount has to be visible before the entity visible,
	// this is to ensure that the mount is already in the client's game when the rider is added.
	if (canBeSeenBy && bVisible && e->riding != NULL)
	{
		return tracker->getTracker(e->riding)->isVisible(tracker, sp, true);
	}
	else if (canBeSeenBy && bVisible)	return eVisibility_SeenAndVisible;
	else if (bVisible)					return eVisibility_IsVisible;
	else								return eVisibility_NotVisible;
}

void TrackedEntity::updatePlayer(EntityTracker *tracker, std::shared_ptr<ServerPlayer> sp)
{
    if (sp == e) return;

	eVisibility visibility = this->isVisible(tracker, sp);
	
    if (	visibility == eVisibility_SeenAndVisible
		&&	seenBy.find(sp) == seenBy.end()				)
	{
        seenBy.insert(sp);
		std::shared_ptr<Packet> packet = getAddEntityPacket();
        sp->connection->send(packet);

		xap = e->xd;
		yap = e->yd;
		zap = e->zd;

		std::shared_ptr<Player> plr = dynamic_pointer_cast<Player>(e);
		if (plr != NULL)
		{
			app.DebugPrintf( "TrackedEntity:: Player '%ls' is now visible to player '%ls', %s.\n",
				plr->name.c_str(), sp->name.c_str(),
				(e->riding==NULL?"not riding minecart":"in minecart")
				);
		}

		// 4J Stu brought forward to fix when Item Frames
		if (!e->getEntityData()->isEmpty() && !(dynamic_pointer_cast<AddMobPacket>(packet)))
		{
			sp->connection->send(std::shared_ptr<SetEntityDataPacket>( new SetEntityDataPacket(e->entityId, e->getEntityData(), true)));
		}

        if (this->trackDelta)
		{
            sp->connection->send( std::shared_ptr<SetEntityMotionPacket>( new SetEntityMotionPacket(e->entityId, e->xd, e->yd, e->zd) ) );
        }

		if (e->riding != NULL)
		{
			sp->connection->send(std::shared_ptr<SetRidingPacket>(new SetRidingPacket(e, e->riding)));
		}

        ItemInstanceArray equipped = e->getEquipmentSlots();
        if (equipped.data != NULL)
		{
            for (unsigned int i = 0; i < equipped.length; i++)
			{
                sp->connection->send( std::shared_ptr<SetEquippedItemPacket>( new SetEquippedItemPacket(e->entityId, i, equipped[i]) ) );
            }
        }

        if (dynamic_pointer_cast<Player>(e) != NULL)
		{
            std::shared_ptr<Player> spe = dynamic_pointer_cast<Player>(e);
            if (spe->isSleeping())
			{
                sp->connection->send( std::shared_ptr<EntityActionAtPositionPacket>( new EntityActionAtPositionPacket(e, EntityActionAtPositionPacket::START_SLEEP, Mth::floor(e->x), Mth::floor(e->y), Mth::floor(e->z)) ) );
            }
        }

		if (dynamic_pointer_cast<Mob>(e) != NULL)
		{
			std::shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(e);
			vector<MobEffectInstance *> *activeEffects = mob->getActiveEffects();
			for(AUTO_VAR(it, activeEffects->begin()); it != activeEffects->end(); ++it)
			{
				MobEffectInstance *effect = *it;

				sp->connection->send(std::shared_ptr<UpdateMobEffectPacket>( new UpdateMobEffectPacket(e->entityId, effect) ) );
			}
			delete activeEffects;
		}
    }
	else if (visibility == eVisibility_NotVisible)
	{
		AUTO_VAR(it, seenBy.find(sp));
        if (it != seenBy.end())
		{
            seenBy.erase(it);
            sp->entitiesToRemove.push_back(e->entityId);
        }
    }

}

bool TrackedEntity::canBySeenBy(std::shared_ptr<ServerPlayer> player)
{
	// 4J - for some reason this isn't currently working, and is causing players to not appear until we are really close to them. Not sure
	// what the conflict is between the java & our version, but removing for now as it is causing issues and we shouldn't *really* need it
	// TODO - investigate further

	return true;
//	return player->getLevel()->getChunkMap()->isPlayerIn(player, e->xChunk, e->zChunk);
}

void TrackedEntity::updatePlayers(EntityTracker *tracker, vector<std::shared_ptr<Player> > *players)
{
    for (unsigned int i = 0; i < players->size(); i++)
	{
        updatePlayer(tracker, dynamic_pointer_cast<ServerPlayer>( players->at(i) ) );
    }
}

std::shared_ptr<Packet> TrackedEntity::getAddEntityPacket()
{
	if (e->removed)
	{
		app.DebugPrintf("Fetching addPacket for removed entity - %ls\n", e->getAName().c_str());
	}

	// 4J-PB - replacing with a switch, rather than tons of ifs
	if (dynamic_pointer_cast<Creature>(e) != NULL)
	{
		yHeadRotp = Mth::floor(e->getYHeadRot() * 256 / 360);
		return std::shared_ptr<AddMobPacket>( new AddMobPacket(dynamic_pointer_cast<Mob>(e), yRotp, xRotp, xp, yp, zp, yHeadRotp) );
	}

	switch(e->GetType())
	{
	case eTYPE_ITEMENTITY:
		{
			std::shared_ptr<AddEntityPacket> packet = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::ITEM, 1, yRotp, xRotp, xp, yp, zp) );
			return packet;
		}
		break;
	case eTYPE_SERVERPLAYER:
		{
			std::shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(e);
			PlayerUID xuid = INVALID_XUID;
			PlayerUID OnlineXuid = INVALID_XUID;
			if( player != NULL )
			{
				xuid = player->getXuid();
				OnlineXuid = player->getOnlineXuid();
			}
			// 4J Added yHeadRotp param to fix #102563 - TU12: Content: Gameplay: When one of the Players is idle for a few minutes his head turns 180 degrees.
			return std::shared_ptr<AddPlayerPacket>( new AddPlayerPacket(dynamic_pointer_cast<Player>(e), xuid, OnlineXuid, xp, yp, zp, yRotp, xRotp, yHeadRotp ) );
		}
		break;
	case eTYPE_MINECART:
		{
			std::shared_ptr<Minecart> minecart = dynamic_pointer_cast<Minecart>(e);
			if (minecart->type == Minecart::RIDEABLE) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::MINECART_RIDEABLE, yRotp, xRotp, xp, yp, zp) );
			if (minecart->type == Minecart::CHEST) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::MINECART_CHEST, yRotp, xRotp, xp, yp, zp) );
			if (minecart->type == Minecart::FURNACE) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::MINECART_FURNACE, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_BOAT:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::BOAT, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_ENDERDRAGON:
		{
			yHeadRotp = Mth::floor(e->getYHeadRot() * 256 / 360);
			return std::shared_ptr<AddMobPacket>( new AddMobPacket(dynamic_pointer_cast<Mob>(e), yRotp, xRotp, xp, yp, zp, yHeadRotp ) );
		}
		break;
	case eTYPE_FISHINGHOOK:
		{
			std::shared_ptr<Entity> owner = dynamic_pointer_cast<FishingHook>(e)->owner;
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FISH_HOOK, owner != NULL ? owner->entityId : e->entityId, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_ARROW:
		{
			std::shared_ptr<Entity> owner = (dynamic_pointer_cast<Arrow>(e))->owner;
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::ARROW, owner != NULL ? owner->entityId : e->entityId, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_SNOWBALL:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::SNOWBALL, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_THROWNPOTION:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::THROWN_POTION, ((dynamic_pointer_cast<ThrownPotion>(e))->getPotionValue()), yRotp, xRotp, xp, yp, zp));
		}
		break;
	case eTYPE_THROWNEXPBOTTLE:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::THROWN_EXPBOTTLE, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_THROWNENDERPEARL:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::THROWN_ENDERPEARL, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_EYEOFENDERSIGNAL:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::EYEOFENDERSIGNAL, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_SMALL_FIREBALL:
		{
			std::shared_ptr<SmallFireball> fb = dynamic_pointer_cast<SmallFireball>(e);
			std::shared_ptr<AddEntityPacket> aep = nullptr;
			if (fb->owner != NULL)
			{
				aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::SMALL_FIREBALL, fb->owner->entityId, yRotp, xRotp, xp, yp, zp) );
			}
			else
			{
				aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::SMALL_FIREBALL, 0, yRotp, xRotp, xp, yp, zp) );
			}
			aep->xa = (int) (fb->xPower * 8000);
			aep->ya = (int) (fb->yPower * 8000);
			aep->za = (int) (fb->zPower * 8000);
			return aep;
		}
		break;
	case eTYPE_DRAGON_FIREBALL:
		{
			std::shared_ptr<DragonFireball> fb = dynamic_pointer_cast<DragonFireball>(e);
			std::shared_ptr<AddEntityPacket> aep = nullptr;
			if (fb->owner != NULL)
			{
				aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::DRAGON_FIRE_BALL, fb->owner->entityId, yRotp, xRotp, xp, yp, zp) );
			}
			else
			{
				aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::DRAGON_FIRE_BALL, 0, yRotp, xRotp, xp, yp, zp) );
			}
			aep->xa = (int) (fb->xPower * 8000);
			aep->ya = (int) (fb->yPower * 8000);
			aep->za = (int) (fb->zPower * 8000);
			return aep;
		}
		break;
	case eTYPE_FIREBALL:
		{
			std::shared_ptr<Fireball> fb = dynamic_pointer_cast<Fireball>(e);
			std::shared_ptr<AddEntityPacket> aep = nullptr;
			if (fb->owner != NULL)
			{
				aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FIREBALL, fb->owner->entityId, yRotp, xRotp, xp, yp, zp) );
			}
			else
			{
				aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FIREBALL, 0, yRotp, xRotp, xp, yp, zp) );
			}
			aep->xa = (int) (fb->xPower * 8000);
			aep->ya = (int) (fb->yPower * 8000);
			aep->za = (int) (fb->zPower * 8000);
			return aep;
		}
		break;
	case eTYPE_THROWNEGG:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::EGG, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_PRIMEDTNT:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::PRIMED_TNT, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_ENDER_CRYSTAL:
		{
			return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::ENDER_CRYSTAL, yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_FALLINGTILE:
		{
			std::shared_ptr<FallingTile> ft = dynamic_pointer_cast<FallingTile>(e);
            return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FALLING, ft->tile | (ft->data << 16), yRotp, xRotp, xp, yp, zp) );
		}
		break;
	case eTYPE_PAINTING:
		{
			return std::shared_ptr<AddPaintingPacket>( new AddPaintingPacket(dynamic_pointer_cast<Painting>(e)) );
		}
		break;
	case eTYPE_ITEM_FRAME:
		{
			std::shared_ptr<ItemFrame> frame = dynamic_pointer_cast<ItemFrame>(e);
			{

				int ix= (int)frame->xTile;
				int iy= (int)frame->yTile;
				int iz= (int)frame->zTile;
				app.DebugPrintf("eTYPE_ITEM_FRAME xyz %d,%d,%d\n",ix,iy,iz);
			}

			std::shared_ptr<AddEntityPacket> packet = std::shared_ptr<AddEntityPacket>(new AddEntityPacket(e, AddEntityPacket::ITEM_FRAME, frame->dir, yRotp, xRotp, xp, yp, zp));
			packet->x = Mth::floor(frame->xTile * 32.0f);
			packet->y = Mth::floor(frame->yTile * 32.0f);
			packet->z = Mth::floor(frame->zTile * 32.0f);
			return packet;
		}
		break;
	case eTYPE_EXPERIENCEORB:
		{
			return std::shared_ptr<AddExperienceOrbPacket>( new AddExperienceOrbPacket(dynamic_pointer_cast<ExperienceOrb>(e)) );
		}
		break;
	default:
		assert(false);
		break;	
	}
/*
	if (e->GetType() == eTYPE_ITEMENTITY)
	{
		std::shared_ptr<ItemEntity> itemEntity = dynamic_pointer_cast<ItemEntity>(e);
        std::shared_ptr<AddItemEntityPacket> packet = std::shared_ptr<AddItemEntityPacket>( new AddItemEntityPacket(itemEntity, xp, yp, zp) );
        itemEntity->x = packet->x / 32.0;
        itemEntity->y = packet->y / 32.0;
        itemEntity->z = packet->z / 32.0;
        return packet;
    }

    if (e->GetType() == eTYPE_SERVERPLAYER )
	{
		std::shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(e);
		XUID xuid = INVALID_XUID;
		XUID OnlineXuid = INVALID_XUID;
		if( player != NULL )
		{
			xuid = player->getXuid();
			OnlineXuid = player->getOnlineXuid();
		}
		return std::shared_ptr<AddPlayerPacket>( new AddPlayerPacket(dynamic_pointer_cast<Player>(e), xuid, OnlineXuid, xp, yp, zp, yRotp, xRotp ) );
    }
    if (e->GetType() == eTYPE_MINECART)
	{
        std::shared_ptr<Minecart> minecart = dynamic_pointer_cast<Minecart>(e);
        if (minecart->type == Minecart::RIDEABLE) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::MINECART_RIDEABLE, yRotp, xRotp, xp, yp, zp) );
        if (minecart->type == Minecart::CHEST) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::MINECART_CHEST, yRotp, xRotp, xp, yp, zp) );
        if (minecart->type == Minecart::FURNACE) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::MINECART_FURNACE, yRotp, xRotp, xp, yp, zp) );
    }
	if (e->GetType() == eTYPE_BOAT)
	{
        return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::BOAT, yRotp, xRotp, xp, yp, zp) );
    }
    if (dynamic_pointer_cast<Creature>(e) != NULL)
	{
        return std::shared_ptr<AddMobPacket>( new AddMobPacket(dynamic_pointer_cast<Mob>(e), yRotp, xRotp, xp, yp, zp) );
    }
	if (e->GetType() == eTYPE_ENDERDRAGON)
	{
		return std::shared_ptr<AddMobPacket>( new AddMobPacket(dynamic_pointer_cast<Mob>(e), yRotp, xRotp, xp, yp, zp ) );
	}
	if (e->GetType() == eTYPE_FISHINGHOOK)
	{
		std::shared_ptr<Entity> owner = dynamic_pointer_cast<FishingHook>(e)->owner;
		return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FISH_HOOK, owner != NULL ? owner->entityId : e->entityId, yRotp, xRotp, xp, yp, zp) );
    }
    if (e->GetType() == eTYPE_ARROW)
	{
        std::shared_ptr<Entity> owner = (dynamic_pointer_cast<Arrow>(e))->owner;
        return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::ARROW, owner != NULL ? owner->entityId : e->entityId, yRotp, xRotp, xp, yp, zp) );
    }
	if (e->GetType() == eTYPE_SNOWBALL)
	{
        return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::SNOWBALL, yRotp, xRotp, xp, yp, zp) );
    }
	if (e->GetType() == eTYPE_THROWNPOTION)
	{
		return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::THROWN_POTION, ((dynamic_pointer_cast<ThrownPotion>(e))->getPotionValue()), yRotp, xRotp, xp, yp, zp));
	}
	if (e->GetType() == eTYPE_THROWNEXPBOTTLE)
	{
		return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::THROWN_EXPBOTTLE, yRotp, xRotp, xp, yp, zp) );
	}
	if (e->GetType() == eTYPE_THROWNENDERPEARL)
	{
		return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::THROWN_ENDERPEARL, yRotp, xRotp, xp, yp, zp) );
	}
	if (e->GetType() == eTYPE_EYEOFENDERSIGNAL)
	{
		return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::EYEOFENDERSIGNAL, yRotp, xRotp, xp, yp, zp) );
	}
	if (e->GetType() == eTYPE_SMALL_FIREBALL)
	{
		std::shared_ptr<SmallFireball> fb = dynamic_pointer_cast<SmallFireball>(e);
		std::shared_ptr<AddEntityPacket> aep = NULL;
		if (fb->owner != NULL)
		{
			aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::SMALL_FIREBALL, fb->owner->entityId, yRotp, xRotp, xp, yp, zp) );
		}
		else
		{
			aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::SMALL_FIREBALL, 0, yRotp, xRotp, xp, yp, zp) );
		}
		aep->xa = (int) (fb->xPower * 8000);
		aep->ya = (int) (fb->yPower * 8000);
		aep->za = (int) (fb->zPower * 8000);
		return aep;
	}
	if (e->GetType() == eTYPE_FIREBALL)
	{
        std::shared_ptr<Fireball> fb = dynamic_pointer_cast<Fireball>(e);
		std::shared_ptr<AddEntityPacket> aep = NULL;
		if (fb->owner != NULL)
		{
			aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FIREBALL, fb->owner->entityId, yRotp, xRotp, xp, yp, zp) );
		}
		else
		{
			aep = std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FIREBALL, 0, yRotp, xRotp, xp, yp, zp) );
		}
        aep->xa = (int) (fb->xPower * 8000);
        aep->ya = (int) (fb->yPower * 8000);
        aep->za = (int) (fb->zPower * 8000);
        return aep;
    }
	if (e->GetType() == eTYPE_THROWNEGG)
	{
        return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::EGG, yRotp, xRotp, xp, yp, zp) );
    }
	if (e->GetType() == eTYPE_PRIMEDTNT)
	{
        return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::PRIMED_TNT, yRotp, xRotp, xp, yp, zp) );
    }
	if (e->GetType() == eTYPE_ENDER_CRYSTAL)
	{
		return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::ENDER_CRYSTAL, yRotp, xRotp, xp, yp, zp) );
	}
	if (e->GetType() == eTYPE_FALLINGTILE)
	{
        std::shared_ptr<FallingTile> ft = dynamic_pointer_cast<FallingTile>(e);
        if (ft->tile == Tile::sand_Id) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FALLING_SAND, yRotp, xRotp, xp, yp, zp) );
        if (ft->tile == Tile::gravel_Id) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FALLING_GRAVEL, yRotp, xRotp, xp, yp, zp) );
		if (ft->tile == Tile::dragonEgg_Id) return std::shared_ptr<AddEntityPacket>( new AddEntityPacket(e, AddEntityPacket::FALLING_EGG, yRotp, xRotp, xp, yp, zp) );
    }
	if (e->GetType() == eTYPE_PAINTING)
	{
        return std::shared_ptr<AddPaintingPacket>( new AddPaintingPacket(dynamic_pointer_cast<Painting>(e)) );
    }
	if (e->GetType() == eTYPE_ITEM_FRAME) 
	{
		std::shared_ptr<ItemFrame> frame = dynamic_pointer_cast<ItemFrame>(e);
		{

			int ix= (int)frame->xTile;
			int iy= (int)frame->yTile;
			int iz= (int)frame->zTile;
		app.DebugPrintf("eTYPE_ITEM_FRAME xyz %d,%d,%d\n",ix,iy,iz);
		}

		std::shared_ptr<AddEntityPacket> packet = std::shared_ptr<AddEntityPacket>(new AddEntityPacket(e, AddEntityPacket::ITEM_FRAME, frame->dir, yRotp, xRotp, xp, yp, zp));
		packet->x = Mth::floor(frame->xTile * 32.0f);
		packet->y = Mth::floor(frame->yTile * 32.0f);
		packet->z = Mth::floor(frame->zTile * 32.0f);
		return packet;
	}
	if (e->GetType() == eTYPE_EXPERIENCEORB)
	{
        return std::shared_ptr<AddExperienceOrbPacket>( new AddExperienceOrbPacket(dynamic_pointer_cast<ExperienceOrb>(e)) );
    }
	assert(false);
	*/
	return nullptr;
}

void TrackedEntity::clear(std::shared_ptr<ServerPlayer> sp)
{
	AUTO_VAR(it, seenBy.find(sp));
    if (it != seenBy.end())
	{
        seenBy.erase(it);
        sp->entitiesToRemove.push_back(e->entityId);
    }
}
