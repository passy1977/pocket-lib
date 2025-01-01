
use crate::config::Config;
use crate::pods::device::Device;
use log::info;

const APP_TAG: &str = "Session"; 

#[allow(unused)] 
#[derive(Debug)]
pub struct Session {
    pub config: Option<Config>,
    pub device: Option<Device>,
    pub init: bool
}

#[allow(unused)] 
impl Session {
    pub fn new(config_json: Option<String>, config_path: Option<String>) -> Result<Session, &'static str> {
        
        info!(target: APP_TAG, "Starting new session");

        match (config_json, config_path) {
            (json @ None, _) => Err("Config json not defined"),
            (json, path) if !json.clone().unwrap().is_empty() => {

                let mut session = Session { 
                    config: None,
                    device: None,
                    init: false
                };

                match Config::new(path) {
                    Ok(conf) => {

                        match conf.parse(&json.unwrap().to_string()) {
                            Ok(device) => session.device = Some(device),
                            Err(msg) => return Err(msg)
                        }

                        session.config = Some(conf);

                        session.init = true;


                        if let Some(dev) = &session.device {
                            info!(target: APP_TAG, "Session uuid:{}", dev.uuid);
                        }
                        
                        Ok(session)
                    }
                    Err(msg) => Err(msg)
                }

            },
            
            (_, _) => Err("Un handled error"),
        }
    }

    
    pub fn register_device(&mut self, _email: &String, _passwd: &String, config_json: &String) -> Result<(), &'static str> {
        Ok(())
    }

}



#[cfg(test)]
mod tests {
    use log::info;

    use super::*;

    const APP_TAG: &str = "Session test"; 

    const REGISTRATION : &str = r#"
    {
        "uuid": "de10b648-9e9c-4b63-96df-2aa662fe20ba",
        "user_uuid": "1281d20e-4e6f-43d8-9aa5-099039f36206",
        "host": "127.0.0.1",
        "host_pub_key": "fookey"
    }
    "#;

    #[test]
    fn it_works() {
        info!(target: APP_TAG, "Start test");
        let session = Session::new(
            Option::Some(REGISTRATION.to_string()), 
            Option::None
        );

        assert!(session.is_ok());

        //session?.register_device("test@salsi.it", "12345678", config_json)

        //session.unwrap().test_pippo(2);
    }
}
