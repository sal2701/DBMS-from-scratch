#include <stdlib.h>
#include <stdio.h>

#include "bst.h"

// Local functions
static int place_bst_node( struct BST_Node *parent, struct BST_Node *node );
static struct BST_Node *make_bst_node( int key, void *data );

// Root's pointer is passed because root can get modified for the first node
int bst_add_node( struct BST_Node **root, int key, void *data )
{
	struct BST_Node *newnode = NULL;
	struct BST_Node *parent = NULL;
	struct BST_Node *retnode = NULL;
	int status = 0;

	newnode = make_bst_node( key, data);
	if( *root == NULL ){
		*root = newnode;
		status = BST_SUCCESS;
	}
	else{
		status = place_bst_node( *root, newnode );
	}
	return status;
}

struct BST_Node *bst_search( struct BST_Node *root, int key )
{
	struct BST_Node *retval = NULL;

	if( root == NULL ){
		return NULL;
	}
	else if( root->key == key )
		return root;
	else if( key < root->key )
		return bst_search( root->left_child, key );
	else if( key > root->key )
		return bst_search( root->right_child, key );
}

void bst_print( struct BST_Node *root )
{
	if( root == NULL )
		return;
	else{
		printf("%d ", root->key);
		bst_print( root->left_child );
		bst_print( root->right_child );
	}
}

void bst_free( struct BST_Node *root )
{
	if( root == NULL )
		return;
	else{
		bst_free( root->left_child );
		bst_free( root->right_child );
		free(root);
	}
}

void bst_destroy( struct BST_Node *root )
{
	if( root == NULL )
		return;
	else{
		bst_free( root->left_child );
		bst_free( root->right_child );
		free(root->data);
		free(root);
	}
}

static int place_bst_node( struct BST_Node *parent, struct BST_Node *node )
{
	int retstatus;

	if( parent == NULL ){
		return BST_NULL;
	}
	else if( node->key == parent->key ){
		return BST_DUP_KEY;
	}
	else if( node->key < parent->key ){
		if( parent->left_child == NULL ){
			parent->left_child = node;
			return BST_SUCCESS;
		}
		else{
			return place_bst_node( parent->left_child, node );
		}
	}
	else if( node->key > parent->key ){
		if( parent->right_child == NULL ){
			parent->right_child = node;
			return BST_SUCCESS;
		}
		else{
			return place_bst_node( parent->right_child, node );
		}
	}
}

static struct BST_Node *make_bst_node( int key, void *data )
{
	struct BST_Node *newnode;
	newnode = (struct BST_Node *) malloc(sizeof(struct BST_Node));
	newnode->key = key;
	newnode->data = data;
	newnode->left_child = NULL;
	newnode->right_child = NULL;

	return newnode;
}

struct BST_Node * minValueNode(struct BST_Node* node)
{
	struct BST_Node *current = node;

	while (current && current->left_child != NULL)
		current = current->left_child;

	return current;
}

int bst_del_node( struct BST_Node** root, int key)
{
	int status;

	if (root == NULL)
	{
		status = BST_NULL;
	}

	// If the key to be deleted is smaller than the root's key,
	// then it lies in left subtree
	if (key < (*root)->key)
	{
		// (*root)->left_child = deleteNode( &(*root)->left_child), key);
		status = bst_del_node( &(*root)->left_child, key);
	}

	// If the key to be deleted is greater than the root's key,
	// then it lies in right subtree
	else if (key > (*root)->key)
	{
		// (*root)->right_child = deleteNode( &(*root)->right_child, key);
		status = bst_del_node( &(*root)->right_child, key);
	}

	// if key is same as root's key, then This is the node
	// to be deleted
	else
	{
		// node with only one child or no child
		if( (*root)->left_child == NULL)
		{
			// struct BST_Node *temp = (*root)->right_child;
			// free(root);
			// return temp;

			*root = (*root)->right_child;
			status = BST_SUCCESS;
		} 
		else if ( (*root)->right_child == NULL)
		{
			// struct node *temp = (*root)->left_child;
			// free(root);
			// return temp;

			*root = (*root)->left_child;
			status = BST_SUCCESS;
		}
		else
		{
			// node with two children: Get the inorder successor (smallest
			// in the right subtree)
			struct BST_Node* temp = minValueNode( (*root)->right_child);

			// Copy the inorder successor's content to this node 
			(*root)->key = temp->key;

			// Delete the inorder successor
			status = bst_del_node( &(*root)->right_child, temp->key);
		}
	}
	return status;
}