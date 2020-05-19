#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <bitset>
using namespace std;

// Author: Glenn Storm
// Text adventure game Zombie Cruise
//
// Object: survive on a cruise ship with a collection of people faced with a mysterious virus that has been released
//
// Locations: [8] Bridge, Fore Deck, Aft Deck, Ballroom, Lounge, Kitchen, Store Room, Engine Room
// Items: [8] Flare Gun, Fire Extinguisher, Alcohol Bottle, Diving Knife, Spear Gun, Wrench, Cleaver, Fuel Can
// People: [8] Captain, 1st Mate, Chef, Bartender, Mr. Rich, Mrs. Rich, Prof. Smart, Ms. Sass
//
// Game data: Time to Release (0-7 turns), Is Released, Radio Used, S.O.S. Called, Rescue Arrived, Game Over
// Time / Scoring data: Time To Rescue (since call for help, 0-31 turns), Score (0-31)
// Player data: Current Location, Left Equip, Right Equip, Health
// Location data: Location I.D., Exit A, Exit B, Exit C
// Item data: Item I.D., Current Location, Is Equipped, Is Used, Damage
// Character data: People I.D., Current Location, Is Infected, Is Zombie, Target Location
//
// Working Memory Budget : 32 bytes (256 bits)
// Game: 1 byte
// Time / Score: 1 byte
// Player 1 byte
// Locations: 8 bytes
// Items: 13 bytes (1 of each item, plus extra one of 5 items)
// Characters: 8 bytes
//
// Boat Map:
//
//       +-----+
//       |  0  |
// +-----+-----+-----+
// |  2 | 4 | 3 | 1 /
//  \---+---+---+--/
//  x\  7 | 5 | 6 /
//    +---+---+--+
//
// 0= Bridge, 1= Fore Deck, 2= Aft Deck, 3= Ballroom, 4= Lounge, 5= Kitchen, 6= Store Room, 7= Engine Room
// Exits: 0=1-2-4, 1=0-3-6, 2=0-4-7, 3=1-4-5, 4=0-2-3, 5=3-6-7, 6=1-5-7, 7=2-5-6
//
// Player Commands:
// Help, Look, Look At [item/character], Take [item], Drop [item], Inventory,
// Use [item/location feature], Attack [character] With [item],
// Move To [location], Talk To [character]
//
// Player can have up to 2 items at once, Player can move, Player can use items, Items can be moved or used, Characters can move
// Characters can be infected, Infected characters turn zombie, Zombies can move, Zombies can attack player or characters
// Characters infected previous turn 'rest' at location one turn, Infected characters turn zombie next turn
// Characters not infected move from locations with zombies, characters stay in locations with other characters or player
// Zombies target location and take another turn to move, Zombies target exits to follow characters or player
// Attacking Zombies with items does damage, Attacking Characters with items kills them (no points)
// Items Spear Gun, Flare Gun, Fuel Can, Alcohol Bottle are one use only, destroyed after use, auto-dropped at location
//
// Fuel can and alcohol bottle used makes location flammable, Flare gun used in flammable location makes fire (while items exist)
// Characters move from locations with fire (will continually exit if entire ship on fire)
// If Bridge is on fire, player cannot use radio to call S.O.S. without taking 4 damage and dying
// Fire can spread to flammable locations connected with exit (chance per turn)
// Using Fire Extinguisher in location removes items that make fire
//
// Character health = 1
// Zombie health = 3
// Player health = 4
// If Player loses all health, - Player loses -
//
// Item Damage: Flare Gun=3, Fire Extinguisher=2, Alcohol Bottle=1, Diving Knife=2, Spear Gun=3, Wrench=2, Cleaver=2, Fuel Can=1
// = ITEM DAMAGE TABLE =
//  Spear Gun, Flare Gun        : 3
//  Fire Extinguisher, Wrench,
//    Cleaver, Diving Knife     : 2
//  Alcohol Bottle, Fuel Can    : 1
// Fire Damage: 2 per turn stayed at location on fire (characters not infected will always survive fire, by exiting)
//
// Player at Bridge can use radio (takes 2 turns)
// If Player uses radio at Bridge, S.O.S. called and timer started
// If Player survives time to rescue, coast guard arrives
// If <4 Zombies left, rescue successful, - Player wins -
// If >=4 Zombies left, coast guard overcome, - Player loses -
//
// Scoring:
// Character survives to successful rescue: 2 points
// Kill a Zombie: 3 points
// Player survives until rescue: 2 points
// Rescue successful (<4 Zombies at rescue): 5 points
// HIGHEST SCORE POSSIBLE : 24+2+5 = 31
// Score Ranks: <11 "Coward", 11-20 "Survivor", 21-30 "Hero", 31 "Zombie Killer"
//
// Dialog:
// Characters say unique lines to player when talked to
// Characters say unique exchanges when together at location
// Infected Characters say subtle clue to excuse 'rest'
// Zombies say unique mumbled versions of character lines
// Player is silent
//
// Game Flow:
// 1. Introduction - Bon Voyage (8 turns)
// 2. Begin - Outbreak (food or drink?, which character 1st?)
// 3. Middle A - Havok (characters and items)
// 4. Plateau - S.O.S. (timer start)
// 5. Middle B - Survive (combat and hazards)
// 6. Crisis - Rescue? (timer end 32 turns)
// 7. End - Prologue and Scoring
//
// Dialog:
// Character dialog is presented in response to "Talk To" player commands or as a result of two characters in the same location
// Attacked character exclaims as injury or dying words
// Infected character dialog is a subtle reference to needing to stop and rest
// Zombie dialog is a mumbled version of the character's normal dialog to player
// Dialog Format: [player response A] [player response B] [char to char] [affirmative] [negative] [injury] [rest] [zombie speak]
// Captain: "Ahoy there" "If you need anything, just ask" "Is everyone all right?" "Certainly" "I'm afraid not" "Oh!" "I need to rest" "Ahhuugh thuuugh"
// 1st Mate: "Here to help" "At your service" "Can I get anyone anything?" "Yes" "Not really" "Ah!" "Let me sit down" "Huungh tuugh huuughp"
// Chef: "Stay out of my kitchen" "Bon appetit" "Is anyone hungry?" "Oui oui" "No" "Sacre bleu!" "Excuse moi" "Buughn appuughtuuught"
// Bartender: "It's always happy hour" "I'm here to listen" "Another round?" "Yeah" "Nope" "Hey!" "I'm going down" "Uuhts uuhlwuuhs huuuhpugh huugh"
// Mr. Rich: "Bully!" "I thought this was a private cruise" "Do you know how much money I have?" "Of course" "Not at all" "I say!" "I'm okay" "Buuulluughe"
// Mrs. Rich: "Well, I never!" "The service here is awful" "Can you help me?" "Yes indeed" "Certainly not" "My stars!" "I'm feeling faint" ""
// Prof. Smart: "This is fascinating" "I have a theory" "Don't you see what this means?" "I think so" "It's unknowable" "Stay back!" "I need a minute" "Thuughs ughs fuuusuunughtugh"
// Ms. Sass: "Seriously?" "Worst cruise ever" "Could this be any more tragic?" "Whatever" "Like I care" "Ew!" "Just leave me alone" "Surunghsluughee"

int main() {
    return 0;
}
