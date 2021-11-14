// Quote movement function base mod for Freeware Cave Story
// Copyright © 2017 Clownacy
// Copyright © 2019 Cucky

#include <windows.h>

#include "mod_loader.h"

#include "cs.h"

static int player_water_max_dash;
static int player_water_max_move;
static int player_water_gravity1;
static int player_water_gravity2;
static int player_water_jump;
static int player_water_dash1;
static int player_water_dash2;
static int player_water_resist;
static int player_water_wall_sliding_speed;

static int player_max_dash;
static int player_max_move;
static int player_gravity1;
static int player_gravity2;
static int player_jump;
static int player_dash1;
static int player_dash2;
static int player_resist;
static int player_wall_sliding_speed;

static int player_booster_fuel;

static bool enable_walljump_flag;
static int walljump_flag;

static void newMove(bool bKey)
{
	//Player isn't hidden (HMC)
	if (!(Player->cond & 2))
	{
		//Set speeds
		int max_dash;
		int max_move;
		int gravity1;
		int gravity2;
		int jump;
		int dash1;
		int dash2;
		int resist;
		int wall_sliding_speed;

		if (Player->flag & 0x100)
		{
			//Speeds when underwater
			max_dash = player_water_max_dash;
			max_move = player_water_max_move;
			gravity1 = player_water_gravity1;
			gravity2 = player_water_gravity2;
			jump = player_water_jump;
			dash1 = player_water_dash1;
			dash2 = player_water_dash2;
			resist = player_water_resist;
			wall_sliding_speed = player_water_wall_sliding_speed;
		}
		else
		{
			//Normal speeds
			max_dash = player_max_dash;
			max_move = player_max_move;
			gravity1 = player_gravity1;
			gravity2 = player_gravity2;
			jump = player_jump;
			dash1 = player_dash1;
			dash2 = player_dash2;
			resist = player_resist;
			wall_sliding_speed = player_wall_sliding_speed;
		}
		
		//Clear ques flag (? when press down when not behind an npc)
		Player->ques = 0;

		//If not holding the boost button, stop boosting
		if (!bKey)
			Player->boost_sw = 0;

		//Movement on the ground
		if (Player->flag & 8 || Player->flag & 0x10 || Player->flag & 0x20)
		{
			//Cancel boost
			Player->boost_sw = 0;

			//Fuel booster
			Player->boost_cnt = player_booster_fuel;
			
			//If movement is enabled, check for walking
			if (bKey)
			{
				//If not holding down
				if (*gKeyTrg != *gKeyDown || *gKey != *gKeyDown || (Player->cond & 1) == 1 || *g_GameFlags & 4)
				{
					if (*gKey != *gKeyDown)
					{
						//Walk left and right
						if (*gKey & *gKeyLeft && Player->xm > -max_dash)
							Player->xm -= dash1;
						if (*gKey & *gKeyRight && Player->xm < max_dash)
							Player->xm += dash1;

						//Face in held direction
						if (*gKey & *gKeyLeft)
							Player->direct = 0;
						if (*gKey & *gKeyRight)
							Player->direct = 2;
					}
				}
				else
				{
					//Inspect if down is pressed
					Player->cond |= 1;
					Player->ques = 1;
				}
			}

			//Friction
			if (!(Player->cond & 0x20))
			{
				if (Player->xm < 0)
				{
					if (Player->xm <= -resist)
						Player->xm += resist;
					else
						Player->xm = 0;
				}

				if (Player->xm > 0)
				{
					if (Player->xm >= resist)
						Player->xm -= resist;
					else
						Player->xm = 0;
				}
			}
		}
		else
		{
			int onWall;
			//Wall jumping
			if ((enable_walljump_flag == true && checkFlag(walljump_flag)) || (enable_walljump_flag == false))
			{
				if (bKey)
				{
					if (Player->flag & 4 && *gKeyRight) // Right wall
						onWall = 1;
					else if (Player->flag & 1 && *gKeyLeft) // Left wall
						onWall = -1;
					else
						onWall = 0;
					if (onWall != 0) {
						if (*gKeyTrg & *gKeyJump) {
							Player->xm = onWall * -1000;
							Player->ym = -jump / 1.15;
							playSFX(15, 1);
							if (onWall == 1) {
								createEffect(Player->x + (8 * 0x200), Player->y, 13, 0);
								createEffect(Player->x + (8 * 0x200), Player->y, 13, 0);
							}
							else {
								createEffect(Player->x - (8 * 0x200), Player->y, 13, 0);
								createEffect(Player->x - (8 * 0x200), Player->y, 13, 0);
							}

						}
						if (Player->ym > wall_sliding_speed)
							Player->ym = wall_sliding_speed;
						}
					}
			}
			//If input is enabled
			if (bKey)
			{
				//Check if you have the booster, and you've pressed the boost button (also if you have fuel)
				if (Player->equip & 0x21 && *gKeyTrg & *gKeyJump && Player->boost_cnt)
				{
					//If has the booster 0.8, start boosting (and halve y velocity if going downwards enough)
					if (Player->equip & 1)
					{
						Player->boost_sw = 1;

						if (Player->ym > 0x100)
							Player->ym /= 2;
					}

					//If has the booster 2.0, start boosting and set speeds
					if (Player->equip & 0x20)
					{
						if (*gKey & *gKeyUp)
						{
							Player->boost_sw = 2;
							Player->xm = 0;
							Player->ym = -0x5FF;
						}
						else if (*gKey & *gKeyLeft)
						{
							Player->boost_sw = 1;
							Player->ym = 0;
							Player->xm = -0x5FF;
						}
						else if (*gKey & *gKeyRight)
						{
							Player->boost_sw = 1;
							Player->ym = 0;
							Player->xm = 0x5FF;
						}
						else if (*gKey & *gKeyDown)
						{
							Player->boost_sw = 3;
							Player->xm = 0;
							Player->ym = 0x5FF;
						}
						else
						{
							Player->boost_sw = 2;
							Player->xm = 0;
							Player->ym = -0x5FF;
						}
					}
				}

				//Boost turning
				if (*gKey & *gKeyLeft && Player->xm > -max_dash)
					Player->xm -= dash2;
				if (*gKey & *gKeyRight && Player->xm < max_dash)
					Player->xm += dash2;

				//Face held direction
				if (*gKey & *gKeyLeft)
					Player->direct = 0;
				if (*gKey & *gKeyRight)
					Player->direct = 2;
			}

			//If booster 2.0 is equipped, and the boost button isn't held (or ran out of fuel) end boost
			if (Player->equip & 0x20 && Player->boost_sw && (!(*gKey & *gKeyJump) || !Player->boost_cnt))
			{
				//Halve speeds if going horizontally or upwards
				if (Player->boost_sw == 1)
				{
					Player->xm /= 2;
				}
				else if (Player->boost_sw == 2)
				{
					Player->ym /= 2;
				}
			}

			//If out of booster fuel (or jump button released), quit boosting
			if (!Player->boost_cnt || !(*gKey & *gKeyJump))
				Player->boost_sw = 0;
		}

		//Jumping
		if (bKey)
		{
			//Look up if up is held
			Player->up = (*gKey & *gKeyUp) != 0;
			
			//Look down if down is held and in the air
			Player->down = *gKey & *gKeyDown && !(Player->flag & 8);

			//If jump is pressed and on the ground (and not in upwards wind), jump
			if (*gKeyTrg & *gKeyJump
				&& (Player->flag & 8 || Player->flag & 0x10 || Player->flag & 0x20)
				&& !(Player->flag & 0x2000))
			{
				//Set speed and play sound
				Player->ym = -jump;
				playSFX(15, 1);
			}
		}

		//If anything other than down is pressed, stop inspecting
		if (bKey && (*gKeyShot | *gKeyJump | *gKeyUp | *gKeyRight | *gKeyLeft) & *gKey)
			Player->cond &= ~1;

		//Decrease booster fuel
		if (Player->boost_sw && Player->boost_cnt)
			--Player->boost_cnt;

		//Wind
		if (Player->flag & 0x1000)
			Player->xm -= 0x88;
		if (Player->flag & 0x2000)
			Player->ym -= 0x80;
		if (Player->flag & 0x4000)
			Player->xm += 0x88;
		if (Player->flag & 0x8000)
			Player->ym += 0x55;

		//If using booster 2.0
		if (Player->equip & 0x20 && Player->boost_sw)
		{
			//Upwards boost effects
			if (Player->boost_sw == 1)
			{
				if (Player->flag & 5)
					Player->ym = -0x100;

				if (!Player->direct)
					Player->xm -= 0x20;
				if (Player->direct == 2)
					Player->xm += 0x20;

				if (*gKeyTrg & *gKeyJump || Player->boost_cnt % 3 == 1)
				{
					if (!Player->direct)
						createEffect(Player->x + 0x400, Player->y + 0x400, 7, 2);
					if (Player->direct == 2)
						createEffect(Player->x - 0x400, Player->y + 0x400, 7, 0);

					playSFX(113, 1);
				}
			}
			//Horizontal boost effects
			else if (Player->boost_sw == 2)
			{
				Player->ym -= 32;

				if (*gKeyTrg & *gKeyJump || Player->boost_cnt % 3 == 1)
				{
					createEffect(Player->x, Player->y + 0xC00, 7, 3);
					playSFX(113, 1);
				}
			}
			//Downwards boost effects
			else if (Player->boost_sw == 3 && (*gKeyTrg & *gKeyJump || Player->boost_cnt % 3 == 1))
			{
				createEffect(Player->x, Player->y - 0xC00, 7, 1);
				playSFX(113, 1);
			}
		}
		//Normal gravity if in upwards wind
		else if (Player->flag & 0x2000)
		{
			Player->ym += gravity1;
		}
		//Booster 0.8 bounce head off ceilings
		else if (Player->equip & 1 && Player->boost_sw && Player->ym > -0x200)
		{
			Player->ym -= 0x20;
			
			if (Player->flag & 2)
				Player->ym = 0x200;
		}
		//Gravity when jump is held
		else if (Player->ym < 0 && bKey && *gKey & *gKeyJump)
		{
			Player->ym += gravity2;
		}
		//Normal gravity
		else
		{
			Player->ym += gravity1;
		}

		//Keep Quote on slopes
		if (bKey != 1 || !(*gKeyTrg & *gKeyJump))
		{
			if (Player->flag & 0x10 && Player->xm < 0)
				Player->ym = -Player->xm;
			if (Player->flag & 0x20 && Player->xm > 0)
				Player->ym = Player->xm;

			if (Player->flag & 8 && Player->flag & 0x80000 && Player->xm < 0)
				Player->ym = 0x400;
			if (Player->flag & 8 && Player->flag & 0x10000 && Player->xm > 0)
				Player->ym = 0x400;
			if (Player->flag & 8 && Player->flag & 0x20000 && Player->flag & 0x40000)
				Player->ym = 0x400;
		}

		//Limit speeds
		if (!(Player->flag & 0x100) || Player->flag & 0xF000)
		{
			//Normal
			if (Player->xm < -max_move)
				Player->xm = -max_move;
			if (Player->xm > max_move)
				Player->xm = max_move;
			if (Player->ym < -max_move)
				Player->ym = -max_move;
			if (Player->ym > max_move)
				Player->ym = max_move;
		}
		else
		{
			//Underwater
			if (Player->xm < -max_move)
				Player->xm = -max_move;
			if (Player->xm > max_move)
				Player->xm = max_move;
			if (Player->ym < -max_move)
				Player->ym = -max_move;
			if (Player->ym > max_move)
				Player->ym = max_move;
		}

		//Was player in water last frame, and they're in water this frame?
		if (!Player->sprash && Player->flag & 0x100)
		{
			//If hitting spike water, make the splash particles red (used in Last Cave)
			int dir = 0;

			if (Player->flag & 0x800)
				dir = 2;
			
			//If on the ground, splash in a different way
			if (Player->flag & 8 || Player->ym <= 0x200)
			{
				if (Player->xm > 0x200 || Player->xm < -0x200)
				{
					//Create particles
					for (int aa = 0; aa < 8; ++aa)
					{
						int xPosition = (random(-8, 8) << 9) + Player->x;
						int yVel = random(-0x200, 0x80);
						int xVel = random(-0x200, 0x200);
						createEntity(73, xPosition, Player->y, xVel + Player->xm, yVel, dir, 0, 0);
					}

					//Play splash SFX
					playSFX(56, 1);
				}
			}
			else
			{
				//Create particles
				for (int a = 0; a < 8; ++a)
				{
					int xPosition = (random(-8, 8) << 9) + Player->x;
					int yVel = random(-0x200, 0x80) - Player->ym / 2;
					int xVel = random(-0x200, 0x200);
					createEntity(73, xPosition, Player->y, xVel + Player->xm, yVel, dir, 0, 0);
				}

				//Play splash SFX
				playSFX(56, 1);
			}

			//Set that we've been in the water
			Player->sprash = 1;
		}
		
		//If no longer in the water, turn off the splash flag
		if (!(Player->flag & 0x100))
			Player->sprash = 0;

		//Damage when you touch a spike
		if (Player->flag & 0x400)
			DamageMyChar(10);

		//** Camera **//
		//Face in direction player is looking horizontally
		if (Player->direct)
		{
			Player->index_x += 0x200;
			if (Player->index_x > 0x8000)
				Player->index_x = 0x8000;
		}
		else
		{
			Player->index_x -= 0x200;
			if (Player->index_x < -0x8000)
				Player->index_x = -0x8000;
		}
		
		//Shift up and down when looking up and down
		if (*gKey & *gKeyUp && bKey)
		{
			Player->index_y -= 0x200;
			if (Player->index_y < -0x8000)
				Player->index_y = -0x8000;
		}
		else if (*gKey & *gKeyDown && bKey)
		{
			Player->index_y += 0x200;
			if (Player->index_y > 0x8000)
				Player->index_y = 0x8000;
		}
		else
		{
			//Move back by itself when neither are held
			if (Player->index_y > 0x200)
				Player->index_y -= 0x200;
			if (Player->index_y < -0x200)
				Player->index_y += 0x200;
		}

		//Set real camera position
		Player->tgt_x = Player->x + Player->index_x;
		Player->tgt_y = Player->y + Player->index_y;

		//Move player
		if (Player->xm > resist || Player->xm < -resist)
			Player->x += Player->xm;
		Player->y += Player->ym;
	}
	
	return;
}

void InitMod(void)
{
	player_water_max_dash = ModLoader_GetSettingInt("player_water_max_dash", 406);
	player_water_max_move = ModLoader_GetSettingInt("player_water_max_move", 767);
	player_water_gravity1 = ModLoader_GetSettingInt("player_water_gravity1", 40);
	player_water_gravity2 = ModLoader_GetSettingInt("player_water_gravity2", 16);
	player_water_jump = ModLoader_GetSettingInt("player_water_jump", 640);
	player_water_dash1 = ModLoader_GetSettingInt("player_water_dash1", 42);
	player_water_dash2 = ModLoader_GetSettingInt("player_water_dash2", 16);
	player_water_resist = ModLoader_GetSettingInt("player_water_resist", 25);
	player_water_wall_sliding_speed = ModLoader_GetSettingInt("player_water_wall_sliding_speed", 554);

	player_max_dash = ModLoader_GetSettingInt("player_max_dash", 812);
	player_max_move = ModLoader_GetSettingInt("player_max_move", 1535);
	player_gravity1 = ModLoader_GetSettingInt("player_gravity1", 80);
	player_gravity2 = ModLoader_GetSettingInt("player_gravity2", 32);
	player_jump = ModLoader_GetSettingInt("player_jump", 1280);
	player_dash1 = ModLoader_GetSettingInt("player_dash1", 85);
	player_dash2 = ModLoader_GetSettingInt("player_dash2", 32);
	player_resist = ModLoader_GetSettingInt("player_resist", 51);
	player_wall_sliding_speed = ModLoader_GetSettingInt("player_wall_sliding_speed", 554);

	player_booster_fuel = ModLoader_GetSettingInt("player_booster_fuel", 50);

	enable_walljump_flag = ModLoader_GetSettingBool("enable_walljump_when_flag_id_is_set", true);
	walljump_flag = ModLoader_GetSettingInt("walljump_flag_id", 4999);

	ModLoader_WriteJump((void*)0x4156C0, (void*)newMove);
}
