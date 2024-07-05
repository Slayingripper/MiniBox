// Challenges.h

#ifndef CHALLENGES_H
#define CHALLENGES_H

// Define challenge levels
enum ChallengeLevel {
  EASY,
  MEDIUM,
  HARD
};

// Function to get the current challenge level based on switch position
ChallengeLevel getCurrentChallengeLevel(int switchPin);

#endif // CHALLENGES_H
