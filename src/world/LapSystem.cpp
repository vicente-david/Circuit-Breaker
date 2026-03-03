// Communicates with a higher level to determine lap count
// Depends on spark position
// will need to store all entities and their associated checkpoints/laps
// but guess what, we have an ecs system, we'll just have it store all entities
// have a component called like Lap
// and do it based off of that