pub mod user;

use sqlite3::Connection;



const CREATION_SQL : &str = r#"
CREATE TABLE `user` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, user_uuid TEXT NOT NULL DEFAULT '', status integer NOT NULL DEFAULT '0');
CREATE TABLE `properties` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `_key` TEXT NOT NULL DEFAULT '', `_value` TEXT NOT NULL DEFAULT '');
CREATE TABLE fields ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, `group_field_id` integer NOT NULL DEFAULT 0, `title` text NOT NULL, `value` text NOT NULL, `is_hidden` integer NOT NULL, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', FOREIGN KEY (user_id) REFERENCES addresses (id));
CREATE TABLE group_fields (id integer primary key autoincrement, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, title text not null, is_hidden integer not null, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', is_temporary integer, FOREIGN KEY (user_id) REFERENCES addresses (id));
CREATE TABLE groups ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, group_id integer, server_group_id integer, `title` text NOT NULL, `icon` text NOT NULL DEFAULT 'UNUSED', `_note` text, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', shared integer, FOREIGN KEY (user_id) REFERENCES addresses (id));
CREATE INDEX `groups_title` ON `groups` (title);
CREATE INDEX `group_fields_group_id` ON `group_fields` (`group_id`);
CREATE INDEX fields_group_field_id ON fields (group_field_id);
CREATE INDEX groups_server_id ON groups (server_id);
CREATE INDEX group_fields_server_id ON group_fields (server_id);
CREATE INDEX fields_server_id ON fields (server_id);
CREATE INDEX groups_user_id ON groups (user_id);
CREATE INDEX group_fields_user_id ON group_fields (user_id);
CREATE INDEX fields_user_id ON fields (user_id);
CREATE INDEX groups_deleted ON groups (deleted);
CREATE INDEX group_fields_deleted ON group_fields (deleted);
CREATE INDEX fields_deleted ON fields (deleted);
"#;


pub struct Database {
    connection: Option<Connection>,
    init: bool
}

impl std::fmt::Debug for Database {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        Ok(())
    }
}


impl Database {
    pub fn new() -> Self {
        Database {
            connection: None,
            init:false
        }
    } 

    pub fn init(&mut self, mut file_db_path: String) -> bool {

        if let Ok(connection) = sqlite3::open(&file_db_path) {
            self.connection = Some(connection);
        } else {
          return false  
        }

        if !self.is_created() {
            self.init = self.create(&file_db_path);
            if !self.init  {
                return false;
            }
        }
        self.init = self.load(&file_db_path);
        
        self.init
    } 

    fn load(&mut self, _file_db_path: &String) -> bool {
        todo!("Check file version")
    }

    fn create(&self, _file_db_path: &String) -> bool {
        if let Some(ref connection) = self.connection {
            if let Ok(()) = connection.execute(CREATION_SQL) {
                return true
            }
        }
        false
    }

    fn is_created(&self) -> bool {
        if let Some(ref connection) = self.connection {
            connection.iterate("SELECT * FROM user", | _ | true ).is_ok()
        } else {
            false
        }
    }

}