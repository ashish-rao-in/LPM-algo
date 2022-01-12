Version 0.0
Updated 08/Jan/2022
Ashish Rao

# LPM
Arrcus LPM take-home

This project is to implements a Routing table using LPM (Longest Prefix Match) algorithm.

# Data structure: 
The LPM will be implemented using a binary tree whose depth is max 32.


# Algorithm:
User can perform 3 actions:
1. Add Route      - Add route to the routing table
2. Delete Route   - Delete route from the routing table
3. Register IP    - Create a cache with a registered IP that gives back nexthop

# Add/Delete
Add/delete functions take the static route info from the user and insert/delete it from the LPM tree respectively.
It converts the ip address into binary data, this binary info is then inserted into the LPM tree. 
The leaf node contains the nexthop information. All other nodes contain -1 (to signify invalid entry).
Detailed information of APIs can be found in lpm.h.

The delete function deletes the leaf node if the children are not present else, it just marks the nexthop to -1 (i.e. non leaf node)

# Register IP
This function populates cache memory with a registed IP and the corresponding nexthop.
It automatically deregisters the previously registered IP when registering the new IP.

The cache is updated whenever a route is added, deleted, or if user registes a new ip address.


