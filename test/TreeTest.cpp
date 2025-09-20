/***************************************************************************
 *
 * Pocket
 * Copyright (C) 2018/2025 Antonio Salsi <passy.linux@zresa.it>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>

#include "pocket/tree.hpp"

using namespace pocket;

class TreeTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        tree_instance = std::make_unique<tree>();
        
        // Create test groups with different hierarchy levels
        group1 = std::make_unique<pods::group>();
        group1->id = 1;
        group1->group_id = 0; // Root level
        group1->title = "Group 1";
        
        group2 = std::make_unique<pods::group>();
        group2->id = 2;
        group2->group_id = 1; // Child of group1
        group2->title = "Group 2";
        
        group3 = std::make_unique<pods::group>();
        group3->id = 3;
        group3->group_id = 1; // Another child of group1
        group3->title = "Group 3";
        
        group4 = std::make_unique<pods::group>();
        group4->id = 4;
        group4->group_id = 2; // Child of group2 (grandchild of group1)
        group4->title = "Group 4";
    }

    void TearDown() override 
    {
        tree_instance.reset();
    }

    std::unique_ptr<tree> tree_instance;
    pods::group::ptr group1, group2, group3, group4;
};

TEST_F(TreeTest, EmptyTreeGet) 
{
    auto result = tree_instance->get();
    EXPECT_TRUE(result.empty());
}

TEST_F(TreeTest, AddSingleGroup) 
{
    bool result = tree_instance->operator+(group1);
    EXPECT_TRUE(result);
    
    auto groups = tree_instance->get();
    EXPECT_EQ(groups.size(), 1);
    EXPECT_EQ(groups[0]->id, 1);
    EXPECT_EQ(groups[0]->title, "Group 1");
}

TEST_F(TreeTest, AddMultipleGroupsFlat) 
{
    // Add groups at the same level (all with group_id = 0)
    group2->group_id = 0;
    group3->group_id = 0;
    
    EXPECT_TRUE(tree_instance->operator+(group1));
    EXPECT_TRUE(tree_instance->operator+(group2));
    EXPECT_TRUE(tree_instance->operator+(group3));
    
    auto groups = tree_instance->get();
    EXPECT_EQ(groups.size(), 3);
}

TEST_F(TreeTest, AddGroupsHierarchy) 
{
    // Add groups in hierarchical order
    EXPECT_TRUE(tree_instance->operator+(group1)); // Root
    EXPECT_TRUE(tree_instance->operator+(group2)); // Child of group1
    EXPECT_TRUE(tree_instance->operator+(group3)); // Another child of group1
    EXPECT_TRUE(tree_instance->operator+(group4)); // Child of group2
    
    auto groups = tree_instance->get();
    EXPECT_EQ(groups.size(), 4);
    
    // Verify the hierarchical structure is maintained
    // The tree should organize groups by levels
    bool found_root = false;
    bool found_children = false;
    bool found_grandchild = false;
    
    for (const auto& group : groups) {
        if (group->id == 1 && group->group_id == 0) {
            found_root = true;
        }
        if ((group->id == 2 || group->id == 3) && group->group_id == 1) {
            found_children = true;
        }
        if (group->id == 4 && group->group_id == 2) {
            found_grandchild = true;
        }
    }
    
    EXPECT_TRUE(found_root);
    EXPECT_TRUE(found_children);
    EXPECT_TRUE(found_grandchild);
}

TEST_F(TreeTest, AddGroupsOutOfOrder) 
{
    // Add groups out of hierarchical order (children before parents)
    EXPECT_TRUE(tree_instance->operator+(group4)); // Grandchild first
    EXPECT_TRUE(tree_instance->operator+(group2)); // Child second
    EXPECT_TRUE(tree_instance->operator+(group1)); // Root last
    EXPECT_TRUE(tree_instance->operator+(group3)); // Another child
    
    auto groups = tree_instance->get();
    EXPECT_EQ(groups.size(), 4);
}

// TEST_F(TreeTest, AddNullGroup) 
// {
//     pods::group::ptr null_group = nullptr;
//     bool result = tree_instance->operator+(null_group);
//     EXPECT_FALSE(result);
//     
//     auto groups = tree_instance->get();
//     EXPECT_TRUE(groups.empty());
// }

TEST_F(TreeTest, AddDuplicateGroup) 
{
    // Add the same group twice
    EXPECT_TRUE(tree_instance->operator+(group1));
    
    // Create another group with the same ID
    auto duplicate_group = std::make_unique<pods::group>();
    duplicate_group->id = 1;
    duplicate_group->group_id = 0;
    duplicate_group->title = "Duplicate Group";
    
    // Adding duplicate should still work but may overwrite
    bool result = tree_instance->operator+(duplicate_group);
    EXPECT_TRUE(result);
    
    auto groups = tree_instance->get();
    EXPECT_EQ(groups.size(), 1);
}

TEST_F(TreeTest, ComplexHierarchy) 
{
    // Create a more complex hierarchy
    auto group5 = std::make_unique<pods::group>();
    group5->id = 5;
    group5->group_id = 0; // Another root
    group5->title = "Group 5";
    
    auto group6 = std::make_unique<pods::group>();
    group6->id = 6;
    group6->group_id = 5; // Child of group5
    group6->title = "Group 6";
    
    auto group7 = std::make_unique<pods::group>();
    group7->id = 7;
    group7->group_id = 4; // Child of group4 (great-grandchild of group1)
    group7->title = "Group 7";
    
    // Add all groups
    EXPECT_TRUE(tree_instance->operator+(group1));
    EXPECT_TRUE(tree_instance->operator+(group2));
    EXPECT_TRUE(tree_instance->operator+(group3));
    EXPECT_TRUE(tree_instance->operator+(group4));
    EXPECT_TRUE(tree_instance->operator+(group5));
    EXPECT_TRUE(tree_instance->operator+(group6));
    EXPECT_TRUE(tree_instance->operator+(group7));
    
    auto groups = tree_instance->get();
    EXPECT_EQ(groups.size(), 7);
    
    // Verify we have multiple root nodes
    int root_count = 0;
    for (const auto& group : groups) {
        if (group->group_id == 0) {
            root_count++;
        }
    }
    EXPECT_EQ(root_count, 2); // group1 and group5
}

TEST_F(TreeTest, TreeStructurePreservation) 
{
    // Add groups and verify the tree maintains the hierarchical structure
    EXPECT_TRUE(tree_instance->operator+(group1));
    EXPECT_TRUE(tree_instance->operator+(group2));
    EXPECT_TRUE(tree_instance->operator+(group3));
    EXPECT_TRUE(tree_instance->operator+(group4));
    
    auto groups = tree_instance->get();
    
    // The tree should maintain parent-child relationships
    // Find parent and verify its children are included
    pods::group::ptr parent = nullptr;
    std::vector<int64_t> child_ids;
    
    for (const auto& group : groups) {
        if (group->id == 1) {
            // Can't copy unique_ptr, just store the ID for verification
        }
        if (group->group_id == 1) {
            child_ids.push_back(group->id);
        }
    }
    
    EXPECT_EQ(child_ids.size(), 2); // group2 and group3
}

TEST_F(TreeTest, GetAfterMultipleOperations) 
{
    // Perform multiple add operations and verify consistency
    EXPECT_TRUE(tree_instance->operator+(group1));
    auto groups1 = tree_instance->get();
    EXPECT_EQ(groups1.size(), 1);
    
    EXPECT_TRUE(tree_instance->operator+(group2));
    auto groups2 = tree_instance->get();
    EXPECT_EQ(groups2.size(), 2);
    
    EXPECT_TRUE(tree_instance->operator+(group3));
    auto groups3 = tree_instance->get();
    EXPECT_EQ(groups3.size(), 3);
    
    EXPECT_TRUE(tree_instance->operator+(group4));
    auto groups4 = tree_instance->get();
    EXPECT_EQ(groups4.size(), 4);
}